#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

typedef struct EImage EImage;

#ifdef EOBJECT_FULL_SCOPE
    struct EImage
    {
        EObject* core; // Access the root EmiObject properties through this reference
        uint8_t innerType; // Internal type ID for NuklearUI, do not modify
        void (*Render)(EImage* self, ETransform* t); // Internal render function, do not invoke
        void (*_free_func)(EImage* self); // Internal free function, do not invoke
    #ifndef RELEASE
        void (*_serialize_func)(BufferWriter* writer, EImage* self); // Internal serialize function, do not invoke
        char* _loadedTexturePath;
    #endif
        Texture2D _loadedTexture; // Internal stored texture
        bool isTextureValid;

        void (*SetTexture)(EImage* self, const char* texturePath);
        Color BackgroundColor; // Background color
        Color ImageColor; // Image color tint
    };
#else
    struct EImage
    {
        EObject* core; // Access the root EmiObject properties through this reference
    #ifndef RELEASE
        uint8_t _reserved[sizeof(uint8_t) + sizeof(void*) * 3 + sizeof(Texture2D) + sizeof(bool) + sizeof(char*)];
    #else
        uint8_t _reserved[sizeof(uint8_t) + sizeof(void*) * 2 + sizeof(Texture2D) + sizeof(bool)];
    #endif

        void (*SetTexture)(EImage* self, const char* texturePath);
        Color BackgroundColor; // Background color
        Color ImageColor; // Image color tint
    };
#endif

EImage* EImage_Create(EObject* parent);