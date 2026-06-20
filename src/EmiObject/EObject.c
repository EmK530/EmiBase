#include <math.h>
#include <stdio.h>

#include "EmiObject/EmiObject.h"
#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"
#include "Libraries/LinkedList.h"

void _eobject_internal_setparent(EObject* ctx, EObject* parent)
{
    if(parent != NULL)
    {
        if(ctx->Parent != NULL)
        {
            Node* node = LinkedList_find(ctx->Parent->Children, ctx);
            if(node != NULL)
                LinkedList_remove(ctx->Parent->Children, node, NULL);
        } else {
            Node* node = LinkedList_find(root_objects, ctx);
            if(node != NULL)
                LinkedList_remove(root_objects, node, NULL);
        }
        ctx->Parent = parent;
        LinkedList_append(parent->Children, ctx);
    } else {
        if(ctx->Parent != NULL)
        {
            Node* node = LinkedList_find(ctx->Parent->Children, ctx);
            if(node != NULL)
                LinkedList_remove(ctx->Parent->Children, node, NULL);
            ctx->Parent = NULL;
            LinkedList_append(root_objects, ctx);
        }
    }
}

void _eobject_internal_destroy(EObject* ctx)
{
    (void)ctx;
}

void _eobject_internal_render(EObject* ctx, ETransform* parent, ETransform* out)
{
    if(!ctx->Visible || ctx->_item == NULL)
        return;
        
    float xScale = 0.0f;
    if(ctx->Size.X.Scale < 0.0f)
    {
        xScale = ctx->Size.Y.Scale * parent->Size.y * -ctx->Size.X.Scale;
    } else {
        xScale = ctx->Size.X.Scale * parent->Size.x;
    }
    out->Size.x = ctx->Size.X.Offset + xScale;
    out->Size.y = ctx->Size.Y.Offset + ctx->Size.Y.Scale * parent->Size.y;

    float localX = ctx->Position.X.Offset + ctx->Position.X.Scale * parent->Size.x;
    float localY = ctx->Position.Y.Offset + ctx->Position.Y.Scale * parent->Size.y;

    localX -= parent->Size.x * parent->Anchor.x;
    localY -= parent->Size.y * parent->Anchor.y;

    // anchor fix
    localX += out->Size.x * (0.5f - ctx->Anchor.x);
    localY += out->Size.y * (0.5f - ctx->Anchor.y);

    float s = sinf(SDEG2RAD(parent->Rotation));
    float c = cosf(SDEG2RAD(parent->Rotation));

    float rx = localX * c - localY * s;
    float ry = localX * s + localY * c;

    out->Position.x = parent->Position.x + rx;
    out->Position.y = parent->Position.y + ry;

    out->Rotation = parent->Rotation + ctx->Rotation;
    out->Anchor = ctx->Anchor;

    ctx->_item->Render(ctx->_item, out);
}

/*
    Creates a new blank EmiObject. This is internal and should not be used manually.
    Set parent to NULL to create as a root object.
*/
EObject* EObject_Create(void* generic)
{
    if(!generic)
    {
        printf("Received null or invalid generic pointer for EObject_Create\n");
        return NULL;
    }
    EObject* object = (EObject*)MemAlloc(sizeof(EObject));
    if(!object)
    {
        printf("Out of memory allocating EObject\n");
        return NULL;
    }

    object->Position = UDim2_new(0.0f, 100, 0.0f, 100);
    object->Size = UDim2_new(0.0f, 100, 0.0f, 100);
    object->Anchor = Vector2_new(0.0f, 0.0f);
    object->Rotation = 0.0f;
    object->Parent = NULL;
    object->Children = LinkedList_create();
    object->Visible = true;
    object->ZIndex = 0;

    object->SetParent = _eobject_internal_setparent;
    object->Destroy = _eobject_internal_destroy;
    object->_render = _eobject_internal_render;
    
    object->_item = (EGeneric*)generic;

    LinkedList_append(root_objects, object);

    return object;
}