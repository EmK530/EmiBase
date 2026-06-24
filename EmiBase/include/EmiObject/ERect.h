#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

typedef struct ERect ERect;

struct ERect
{
    EObject* core; // Access the root EmiObject properties through this reference
    void (*Render)(ERect* self, ETransform* t); // Internal render function, do not invoke
    void (*free_func)(ERect* self); // Internal free function, do not invoke
    void (*serialize_func)(ERect* self); // Internal serialize function, do not invoke

    Color Color; // Rectangle color
};

ERect* ERect_Create(EObject* parent);