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

/*
    Creates a new Rect EmiObject.
    Set parent to NULL to create as a root object.
*/
ERect* ERect_Create(EObject* parent)
{
    ERect* rect = (ERect*)MemAlloc(sizeof(ERect));
    if(!rect)
    {
        printf("Out of memory allocating ERect\n");
        return NULL;
    }

    rect->core = EObject_Create(rect);
    rect->free_func = NULL;
    rect->Render = _erect_internal_render;
    
    rect->Color.r = 255;
    rect->Color.g = 255;
    rect->Color.b = 255;
    rect->Color.a = 255;

    if(parent != NULL)
        rect->core->SetParent(rect->core, parent);

    return rect;
}