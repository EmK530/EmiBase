#include <stdio.h>

#define EMIBASE_INTERNAL

#include "EmiBase.h"

void _erect_internal_render(ERect* rect, ETransform* t)
{
    Color color = { rect->Color.r, rect->Color.g, rect->Color.b, rect->Color.a };
    if(rect->Color.a == 0) return;

    Rectangle r = { t->Position.x, t->Position.y, t->Size.x, t->Size.y };
    Vector2 origin = { t->Size.x * 0.5f, t->Size.y * 0.5f };
    DrawRectanglePro(r, origin, t->Rotation, color);
}

#ifndef RELEASE
    void _erect_internal_serialize(BufferWriter* writer, ERect* self)
    {
        BW_WriteU8(writer, EObjectType_ERect); // Type identifier
        Color32_serialize(writer, self->Color);
    }
#endif

extern void _eobject_internal_initialize(EObject* object);

/*
    Creates a new Rect EmiObject.
    Set parent to NULL to create as a root object.
*/
ERect* ERect_Create(EObject* parent)
{
    ERect* rect = (ERect*)ecalloc_strict(1, sizeof(ERect));

    rect->innerType = EObjectType_ERect;
    _eobject_internal_initialize((EObject*)rect);

    rect->_free_func = NULL;
    rect->_render = (void(*)(EObject*, ETransform*))_erect_internal_render;
#ifndef RELEASE
    rect->_serialize_func = (void(*)(BufferWriter*, EObject*))_erect_internal_serialize;
#endif
    EObject_SetName(rect, "ERect");
    
    rect->Color.r = 255;
    rect->Color.g = 255;
    rect->Color.b = 255;
    rect->Color.a = 255;

    if(parent != NULL)
        EObject_SetParent(rect, parent);

    return rect;
}