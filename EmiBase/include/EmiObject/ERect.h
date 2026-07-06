#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

typedef struct
{
    EOBJECT_BASE_TYPES
    Color Color; // Rectangle color
} ERect;

ERect* ERect_Create(EObject* parent);