#include <stdio.h>
#include <string.h>

#define EMIBASE_INTERNAL

#include "EmiBase.h"
#include "rlgl.h"

void _eimage_internal_render(EImage* image, ETransform* t)
{
#if SOFTWARE_OPTIMIZATIONS == 1
    if(image->BackgroundColor.a > 127)
    {
        image->BackgroundColor.a = 255;
#else
    if(image->BackgroundColor.a > 2)
    {
#endif
        Rectangle r = { t->Position.x, t->Position.y, t->Size.x, t->Size.y };
        Vector2 origin = { t->Size.x * 0.5f, t->Size.y * 0.5f };
        DrawRectanglePro(r, origin, t->Rotation, image->BackgroundColor);
    }
    if(image->isTextureValid)
    {
        Rectangle src = { 0, 0, image->_loadedTexture.width, image->_loadedTexture.height };
        Rectangle r = { t->Position.x, t->Position.y, t->Size.x, t->Size.y };
        Vector2 origin = { t->Size.x * 0.5f, t->Size.y * 0.5f };
        DrawTexturePro(image->_loadedTexture, src, r, origin, t->Rotation, image->ImageColor);
    }
}

void EImage_SetTexture(EImage* image, const char* texturePath)
{
    size_t dataSize = strlen(texturePath) + 1;
#ifndef RELEASE
    if(image->_loadedTexturePath != NULL)
        MemFree(image->_loadedTexturePath);
    image->_loadedTexturePath = MemAlloc(dataSize);
    if(image->_loadedTexturePath != NULL)
        memcpy(image->_loadedTexturePath, texturePath, dataSize);
#endif
    if(image->isTextureValid)
    {
        UnloadTexture(image->_loadedTexture);
        image->isTextureValid = false;
    }
    Texture2D loaded = ContentManager_LoadTexture(texturePath);
    if(IsTextureValid(loaded))
    {
        image->_loadedTexture = loaded;
        image->isTextureValid = true;
    }
}

#ifndef RELEASE
    void _eimage_internal_serialize(BufferWriter* writer, EImage* self)
    {
        BW_WriteU8(writer, EObjectType_EImage); // Type identifier
        uint8_t pathSize = strlen(self->_loadedTexturePath);
        BW_WriteU8(writer, pathSize);
        BW_WriteString(writer, self->_loadedTexturePath, pathSize);
        Color32_serialize(writer, self->BackgroundColor);
        Color32_serialize(writer, self->ImageColor);
    }
#endif

extern void _eobject_internal_initialize(EObject* object);

#ifndef RELEASE
    void _eimage_internal_free(EObject* item)
    {
        EImage* image = (EImage*)item;
        MemFree(image->_loadedTexturePath);
        image->_loadedTexturePath = NULL;
        if(image->isTextureValid)
            UnloadTexture(image->_loadedTexture);
    }
#endif

/*
    Creates a new Image EmiObject.
    Set parent to NULL to create as a root object.
*/
EImage* EImage_Create(EObject* parent)
{
    EImage* image = (EImage*)MemAlloc(sizeof(EImage));
    if(!image)
    {
        eprintf("Out of memory allocating EImage\n");
        return NULL;
    }

    image->innerType = EObjectType_EImage;
    _eobject_internal_initialize((EObject*)image);

    image->_render = (void(*)(EObject*, ETransform*))_eimage_internal_render; // swagging on that unsafe cast
#ifndef RELEASE
    image->_serialize_func = (void(*)(BufferWriter*, EObject*))_eimage_internal_serialize;
    image->_loadedTexturePath = NULL;
    image->_free_func = _eimage_internal_free;
#else
    image->_free_func = NULL;
#endif
    EObject_SetName(image, "EImage");
    EImage_SetTexture(image, DEFAULT_IMAGE);
    
    image->BackgroundColor = Color32_new(255, 255, 255, 0);
    image->ImageColor = Color32_new(255, 255, 255, 255);

    if(parent != NULL)
        EObject_SetParent(image, parent);

    return image;
}