#include <stdio.h>
#include <string.h>
#include "EmiObject/EmiObject.h"
#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

LinkedList* root_objects;

int EmiObject_Init()
{
    root_objects = LinkedList_create();
    if(root_objects == NULL)
        return 0;

    eprintf("[EmiObject] Ready!\n");
    return 1;
}

// Draw EObjects recursively
void Recursive_EObject_Draw(EObject* object, ETransform* parent)
{
    ETransform current;

    object->_render(object, parent, &current);

    LinkedList_foreach(object->Children, child)
    {
        EObject* co = (EObject*)child->item;
        Recursive_EObject_Draw(co, &current);
    }
}

void EmiObject_Draw(int screenWidth, int screenHeight)
{
    Vector2 res = { (float)screenWidth, (float)screenHeight };
    ETransform root = {
        .Position = {0.0f, 0.0f},
        .Size     = {res.x, res.y},
        .Rotation = 0.0f,
        .Anchor   = {0.0f, 0.0f}
    };
    LinkedList_foreach(root_objects, obj)
    {
        EObject* object = (EObject*)obj->item;
        Recursive_EObject_Draw(object, &root);
    }
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
            case 1:
            {
                ERect* rect = ERect_Create(parent);
                rect->Color = Color32_deserialize(reader);
                obj = rect->core;
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
        eprintf("[EmiObject] Deserialization failed, invalid header magic.\n");
        return;
    }
    MemFree(magic);
    uint8_t version = BR_ReadU8(reader);
    if(version != 1)
    {
        eprintf("[EmiObject] Deserialization failed, unsupported file version: %i\n", version);
        return;
    }
    _internal_deserialize_recursively(reader, NULL);
}

#ifndef RELEASE
    void _internal_serialize_recursively(BufferWriter* writer, EObject* parent, EObject* target)
    {
        // Serialize ERect properties before EObject
        target->_item->_serialize_func(writer, target->_item);
        target->_serialize_func(writer, target);
        BW_WriteU32(writer, target->Children->size);
        LinkedList_foreach(target->Children, child)
        {
            _internal_serialize_recursively(writer, target, child->item);
        }
    }

    void EmiObject_Serialize()
    {
        BufferWriter* writer = BW_CreateWithCapacity(8192);
        BW_WriteString(writer, "EOBJ", 4);
        BW_WriteU8(writer, 1); // Version
        BW_WriteU32(writer, root_objects->size);
        LinkedList_foreach(root_objects, obj)
        {
            EObject* object = (EObject*)obj->item;
            _internal_serialize_recursively(writer, NULL, object);
        }
        BW_SaveToFile(writer, "output.eobj");
    }
#endif