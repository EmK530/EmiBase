#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

typedef struct EImage EImage;

#ifdef EMIBASE_INTERNAL
    struct EImage
    {
        EOBJECT_BASE_TYPES
    #ifndef RELEASE
        char* _loadedTexturePath;
    #endif
        Texture2D _loadedTexture; // Internal stored texture
        bool isTextureValid;

        Color BackgroundColor; // Background color
        Color ImageColor; // Image color tint
    };
#else
    struct EImage
    {
        EOBJECT_BASE_TYPES
    #ifndef RELEASE
        uint8_t _reservedi[sizeof(Texture2D) + sizeof(bool) + sizeof(char*)];
    #else
        uint8_t _reservedi[sizeof(Texture2D) + sizeof(bool)];
    #endif

        Color BackgroundColor; // Background color
        Color ImageColor; // Image color tint
    };
#endif

EImage* EImage_Create(EObject* parent);
void EImage_SetTexture(EImage* image, const char* texturePath);