#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

typedef struct ERect ERect;

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

ERect* ERect_Create(EObject* parent);