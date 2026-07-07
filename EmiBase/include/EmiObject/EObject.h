#pragma once

#include <raylib.h>
#include <stdbool.h>

#include "EmiObject/Types.h"
#include "EmiObject/LinkedObjectList.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#define SDEG2RAD(deg) ((deg) * (M_PI / 180.0))
#define SRAD2DEG(rad) ((rad) * (180.0 / M_PI))

#ifndef RELEASE
    #define _EOBJECT_DEBUG_FIELDS \
        void (*_serialize_func)(BufferWriter* writer, EObject* self); \
        bool _nk_expanded;
#else
    #define _EOBJECT_DEBUG_FIELDS
#endif

enum EObjectType
{
    EObjectType_ERect = 1,
    EObjectType_EImage = 2,
    EObjectType_EText = 3
};

typedef struct
{
    Vector2 Position;
    Vector2 Size;
    float Rotation;
} ETransform;

typedef struct EObject EObject;

#define EOBJECT_BASE_TYPES \
    char* Name; \
    EObject* _parent; \
    EObject* _node_next; \
    EObject* _node_prev; \
    void (*_render)(EObject* ctx, ETransform* t); \
    void (*_free_func)(EObject* ctx); \
    _EOBJECT_DEBUG_FIELDS \
    EUDim2 Position; \
    EUDim2 Size; \
    Vector2 Anchor; \
    LinkedObjectList Children; \
    float Rotation; \
    bool Visible; \
    uint8_t ZIndex; \
    uint8_t innerType;

struct EObject
{
    EOBJECT_BASE_TYPES
};

void EObject_SetParent(void* ctx, EObject* parent);
void EObject_Destroy(void* ctx);
void EObject_SetName(void* object, char* name);