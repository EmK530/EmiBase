#include <stdio.h>
#include <string.h>
#define EMIBASE_INTERNAL
#include "EmiBase/NuklearUI.h"
#include "EmiObject/EmiObject.h"
#include "EmiObject/Types.h"
#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

LinkedObjectList root_objects;

int EmiObject_Init()
{
    root_objects = LinkedObjectList_create();
    eprintf("[EmiObject] Ready!\n");
    return 1;
}

// Draw EObjects recursively
extern void _eobject_internal_render(EObject* ctx, ETransform* parent, ETransform* out);
void Recursive_EObject_Draw(EObject* object, ETransform* parent)
{
    if(object->Parent != object->_ParentInternalTracking)
    {
        eprintf("[EmiObject] Destroying object '%s' with improperly set parent, did you mean to use EObject_SetParent?\n", object->Name);
        EObject_Destroy(object);
        return;
    }
    ETransform current;
    if(!object->Visible)
        return;
    _eobject_internal_render(object, parent, &current);
    LinkedObjectList_foreach(object->Children, co)
        Recursive_EObject_Draw(co, &current);
}

static EVector2i drawing_offset = {.X = 0, .Y = 0};
void EmiObject_SetDrawOffset(EVector2i value)
{
    drawing_offset = value;
}
void EmiObject_Draw(int screenWidth, int screenHeight)
{
    Vector2 res = { (float)screenWidth, (float)screenHeight };
    ETransform root = {
        .Position = {res.x/2.0f + drawing_offset.X, res.y/2.0f + drawing_offset.Y},
        .Size     = {res.x, res.y},
        .Rotation = 0.0f
    };
    LinkedObjectList_foreach(root_objects, object)
        Recursive_EObject_Draw(object, &root);
    drawing_offset = (EVector2i){.X = 0, .Y = 0};
}

void _emiobject_internal_wipe_recursively(LinkedObjectList* collection, EObject* object)
{
    LinkedObjectList_foreach(*collection, child)
        _emiobject_internal_wipe_recursively(&child->Children, child);
    LinkedObjectList_clear(collection);
    if(object != NULL) {
        if(object->_ParentInternalTracking == NULL) {
            LinkedObjectList_remove(&root_objects, object);
        } else {
            LinkedObjectList_remove(&object->_ParentInternalTracking->Children, object);
        }
        if(object == nk_selected_object)
            NuklearUI_ResetHighlight();
        MemFree(object->Name);
        if(object->_free_func != NULL)
            object->_free_func(object);
        MemFree(object);
    }
}

void EmiObject_Wipe()
{
    _emiobject_internal_wipe_recursively(&root_objects, NULL);
    root_objects = LinkedObjectList_create();
}

void _internal_deserialize_recursively(BufferReader* reader, EObject* parent)
{
    uint32_t obj_count = BR_ReadU32(reader);
    for(uint32_t i = 0; i < obj_count; i++)
    {
        uint8_t obj_type = BR_ReadU8(reader);
        uint8_t success = 1;
        EObject* obj = NULL;
        switch(obj_type)
        {
            case EObjectType_ERect:
            {
                ERect* rect = ERect_Create(parent);
                rect->Color = Color32_deserialize(reader);
                obj = (EObject*)rect;
                break;
            }
            case EObjectType_EImage:
            {
                EImage* image = EImage_Create(parent);
                uint8_t pathLen = BR_ReadU8(reader);
                char* path = BR_ReadString(reader, pathLen);
                EImage_SetTexture(image, path);
                MemFree(path);
                image->BackgroundColor = Color32_deserialize(reader);
                image->ImageColor = Color32_deserialize(reader);
                obj = (EObject*)image;
                break;
            }
            default:
                success = 0;
        }
        if(!success)
        {
            eprintf("[EmiObject] Deserialization failed, unknown object type ID: %i\n", obj_type);
            return;
        }

        uint8_t name_len = BR_ReadU8(reader);
        if(name_len != 0)
            obj->Name = BR_ReadString(reader, name_len);
        obj->Position = EUDim2_deserialize(reader);
        obj->Size = EUDim2_deserialize(reader);
        obj->Rotation = BR_ReadFloat(reader);
        obj->Anchor = Vector2_deserialize(reader);
        obj->Visible = BR_ReadU8(reader) == 1;
        obj->ZIndex = BR_ReadU8(reader);
        _internal_deserialize_recursively(reader, obj);
    }
}

void EmiObject_Deserialize(const char* filePath)
{
    size_t size;
    unsigned char* data = ContentManager_LoadFile(filePath, &size);
    if(data == NULL)
    {
        eprintf("[EmiObject] Deserialization failed, could not load file path: '%s'\n", filePath);
        return;
    }
    
    BufferReader* reader = BR_CreateFromMemory(data, size);
    char* magic = BR_ReadString(reader, 4);
    if(strcmp(magic, "EOBJ") != 0)
    {
        MemFree(magic);
        BR_Destroy(reader);
        MemFree(data);
        eprintf("[EmiObject] Deserialization failed, invalid header magic.\n");
        return;
    }
    MemFree(magic);
    uint8_t version = BR_ReadU8(reader);
    if(version != 1)
    {
        BR_Destroy(reader);
        MemFree(data);
        eprintf("[EmiObject] Deserialization failed, unsupported file version: %i\n", version);
        return;
    }
    _internal_deserialize_recursively(reader, NULL);
    BR_Destroy(reader);
    MemFree(data);
}

#ifndef RELEASE
    void _internal_serialize_recursively(BufferWriter* writer, EObject* parent, EObject* target)
    {
        // Serialize ERect properties before EObject
        target->_serialize_func(writer, target);
        target->_serialize_func(writer, target);
        BW_WriteU32(writer, target->Children.size);
        LinkedObjectList_foreach(target->Children, child)
            _internal_serialize_recursively(writer, target, child);
    }

    void EmiObject_Serialize()
    {
        BufferWriter* writer = BW_CreateWithCapacity(8192);
        BW_WriteString(writer, "EOBJ", 4);
        BW_WriteU8(writer, 1); // Version
        BW_WriteU32(writer, root_objects.size);
        LinkedObjectList_foreach(root_objects, object)
            _internal_serialize_recursively(writer, NULL, object);
        BW_SaveToFile(writer, "Workspace.eobj");
    }
#endif