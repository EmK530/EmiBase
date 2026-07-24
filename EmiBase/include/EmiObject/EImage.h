#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

#ifndef EIMAGE_VERSION
    #define EIMAGE_VERSION 1
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
        uint8_t textureState;

        Color BackgroundColor; // Background color
        Color ImageColor; // Image color tint
    };
#else
    struct EImage
    {
        EOBJECT_BASE_TYPES
    #ifndef RELEASE
        uint8_t _reservedi[sizeof(Texture2D) + sizeof(uint8_t) + sizeof(char*)];
    #else
        uint8_t _reservedi[sizeof(Texture2D) + sizeof(uint8_t)];
    #endif

        Color BackgroundColor; // Background color
        Color ImageColor; // Image color tint
    };
#endif

EImage* EImage_Create(EObject* parent);
void EImage_SetTexture(EImage* image, const char* texturePath);