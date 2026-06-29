#pragma once

#include <raylib.h>
#include <stdbool.h>

#include "EmiObject/Types.h"
#include "Libraries/LinkedList.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979
#endif
#define SDEG2RAD(deg) ((deg) * (M_PI / 180.0))
#define SRAD2DEG(rad) ((rad) * (180.0 / M_PI))

typedef struct EObject EObject;
typedef struct EGeneric EGeneric;

typedef struct
{
    Vector2 Position;
    Vector2 Size;
    float Rotation;
} ETransform;

struct EGeneric
{
    EObject* core; // Access the root EmiObject properties through this reference
    uint8_t innerType; // Internal type ID for NuklearUI, do not modify
    void (*Render)(EGeneric* self, ETransform* t); // Internal render function, do not invoke
    void (*_free_func)(EGeneric* self); // Internal free function, do not invoke
#ifndef RELEASE
    void (*_serialize_func)(BufferWriter* writer, EGeneric* self); // Internal serialize function, do not invoke
#endif
};

#ifdef EOBJECT_FULL_SCOPE
    struct EObject
    {
        char* Name; // The name of the object, do not modify directly.
        EUDim2 Position; // The position of the object
        EUDim2 Size; // The size of the object
        float Rotation; // The rotation of the object (degrees)
        Vector2 Anchor; // Anchor point of the object from 0 to 1: 0,0 = top left | 1,1 = bottom right

        LinkedList* Children; // Child objects of this EmiObject
        void (*SetParent)(EObject* ctx, EObject* parent); // Update the parent of this EmiObject
        void (*Destroy)(EObject* ctx); // Delete this EmiObject (TO BE IMPLEMENTED)
        void (*SetName)(EObject* ctx, char* name); // Update the name of this EmiObject, the memory does not need to persist as this will be allocated

        bool Visible; // Should this EmiObject be rendered? Affects child objects
        uint8_t ZIndex; // Layering of this EmiObject for rendering (TO BE IMPLEMENTED)

        EGeneric* _item; // Internal attached object, can be cast to ERect or other types if you know what you are accessing

        // Internals

        EObject* _parent;
        void (*_render)(EObject* ctx, ETransform* parent, ETransform* out);
    #ifndef RELEASE
        void (*_serialize_func)(BufferWriter* writer,EObject* self);
        bool _nk_expanded;
    #endif
    };
#else
    struct EObject
    {
        char* Name; // The name of the object, do not modify directly.
        EUDim2 Position; // The position of the object
        EUDim2 Size; // The size of the object
        float Rotation; // The rotation of the object (degrees)
        Vector2 Anchor; // Anchor point of the object from 0 to 1: 0,0 = top left | 1,1 = bottom right

        LinkedList* Children; // Child objects of this EmiObject
        void (*SetParent)(EObject* ctx, EObject* parent); // Update the parent of this EmiObject
        void (*Destroy)(EObject* ctx); // Delete this EmiObject (TO BE IMPLEMENTED)
        void (*SetName)(EObject* ctx, char* name); // Update the name of this EmiObject, the memory does not need to persist as this will be allocated

        bool Visible; // Should this EmiObject be rendered? Affects child objects
        uint8_t ZIndex; // Layering of this EmiObject for rendering (TO BE IMPLEMENTED)

        EGeneric* _item; // Internal attached object, can be cast to ERect or other types if you know what you are accessing
    };
#endif

EObject* EObject_Create(void* generic);