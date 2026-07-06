#include <stdio.h>
#include <string.h>

#define EOBJECT_FULL_SCOPE

#include "EmiObject/EmiObject.h"
#include "EmiObject/EImage.h"
#include "EmiObject/Types.h"
#include "EmiBase/ContentManager.h"

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

void _eimage_internal_settexture(EImage* image, const char* texturePath)
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
        BW_WriteU8(writer, 2); // Type identifier
        uint8_t pathSize = strlen(self->_loadedTexturePath);
        BW_WriteU8(writer, pathSize);
        BW_WriteString(writer, self->_loadedTexturePath, pathSize);
        Color32_serialize(writer, self->BackgroundColor);
        Color32_serialize(writer, self->ImageColor);
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

    image->core = EObject_Create(image);
    image->_free_func = NULL;
    image->Render = _eimage_internal_render;
#ifndef RELEASE
    image->_serialize_func = _eimage_internal_serialize;
    image->_loadedTexturePath = NULL;
#endif
    image->SetTexture = _eimage_internal_settexture;
    image->core->SetName(image->core, "EImage");
    image->SetTexture(image, "image/EImageDefaultTexture.png");
    
    image->innerType = 2;
    image->BackgroundColor = Color32_new(255, 255, 255, 0);
    image->ImageColor = Color32_new(255, 255, 255, 255);

    if(parent != NULL)
        image->core->SetParent(image->core, parent);

    return image;
}