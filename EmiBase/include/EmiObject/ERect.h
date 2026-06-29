#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

typedef struct ERect ERect;

#ifdef EOBJECT_FULL_SCOPE
    struct ERect
    {
        EObject* core; // Access the root EmiObject properties through this reference
        uint8_t innerType; // Internal type ID for NuklearUI, do not modify
        void (*Render)(ERect* self, ETransform* t); // Internal render function, do not invoke
        void (*_free_func)(ERect* self); // Internal free function, do not invoke
    #ifndef RELEASE
        void (*_serialize_func)(BufferWriter* writer, ERect* self); // Internal serialize function, do not invoke
    #endif

        Color Color; // Rectangle color
    };
#else
    struct ERect
    {
        EObject* core; // Access the root EmiObject properties through this reference
    #ifndef RELEASE
        uint8_t _reserved[sizeof(uint8_t) + sizeof(void*) * 3];
    #else
        uint8_t _reserved[sizeof(uint8_t) + sizeof(void*) * 2];
    #endif

        Color Color; // Rectangle color
    };
#endif

ERect* ERect_Create(EObject* parent);