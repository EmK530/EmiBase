#include <stdio.h>
#include <string.h>

#define EOBJECT_FULL_SCOPE

#include "EmiBase.h"

void _eimage_internal_render(EImage* image, ETransform* t)
{
    if(image->BackgroundColor.a > 0)
    {
        Color color = { image->BackgroundColor.r, image->BackgroundColor.g, image->BackgroundColor.b, image->BackgroundColor.a };
        if (t->Rotation == 0.0f) {
            DrawRectangle((int)(t->Position.x - t->Size.x * 0.5f),(int)(t->Position.y - t->Size.y * 0.5f),(int)t->Size.x,(int)t->Size.y,color);
        } else {
            Rectangle r = { t->Position.x, t->Position.y, t->Size.x, t->Size.y };
            Vector2 origin = { t->Size.x * 0.5f, t->Size.y * 0.5f };
            DrawRectanglePro(r, origin, t->Rotation, color);
        }
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
    if(image->_loadedTexturePath != NULL)
        MemFree(image->_loadedTexturePath);
    size_t dataSize = strlen(texturePath) + 1;
    image->_loadedTexturePath = MemAlloc(dataSize);
    if(image->_loadedTexturePath != NULL)
        memcpy(image->_loadedTexturePath, texturePath, dataSize);
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
    EImage_SetTexture(image, "image/EImageDefaultTexture.png");
    
    image->innerType = EObjectType_EImage;
    image->BackgroundColor = Color32_new(255, 255, 255, 0);
    image->ImageColor = Color32_new(255, 255, 255, 255);

    if(parent != NULL)
        EObject_SetParent(image, parent);

    return image;
}