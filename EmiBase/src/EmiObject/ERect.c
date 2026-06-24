#include <stdio.h>

#include "EmiObject/EmiObject.h"
#include "EmiObject/ERect.h"
#include "EmiObject/Types.h"

void _erect_internal_render(ERect* rect, ETransform* t)
{
    Color color = { rect->Color.r, rect->Color.g, rect->Color.b, rect->Color.a };

    if (t->Rotation == 0.0f)
    {
        DrawRectangle(
            (int)(t->Position.x - t->Size.x * 0.5f),
            (int)(t->Position.y - t->Size.y * 0.5f),
            (int)t->Size.x,
            (int)t->Size.y,
            color
        );
    }
    else
    {
        Rectangle r = { t->Position.x, t->Position.y, t->Size.x, t->Size.y };
        Vector2 origin = { t->Size.x * 0.5f, t->Size.y * 0.5f };
        DrawRectanglePro(r, origin, t->Rotation, color);
    }
}

#ifndef RELEASE
    void _erect_internal_serialize(BufferWriter* writer, ERect* self)
    {
        BW_WriteU8(writer, 1); // Type identifier
        Color32_serialize(writer, self->Color);
    }
#endif

/*
    Creates a new Rect EmiObject.
    Set parent to NULL to create as a root object.
*/
ERect* ERect_Create(EObject* parent)
{
    ERect* rect = (ERect*)MemAlloc(sizeof(ERect));
    if(!rect)
    {
        eprintf("Out of memory allocating ERect\n");
        return NULL;
    }

    rect->core = EObject_Create(rect);
    rect->_free_func = NULL;
    rect->Render = _erect_internal_render;
#ifndef RELEASE
    rect->_serialize_func = _erect_internal_serialize;
#endif
    rect->core->SetName(rect->core, "ERect");
    
    rect->Color.r = 255;
    rect->Color.g = 255;
    rect->Color.b = 255;
    rect->Color.a = 255;

    if(parent != NULL)
        rect->core->SetParent(rect->core, parent);

    return rect;
}