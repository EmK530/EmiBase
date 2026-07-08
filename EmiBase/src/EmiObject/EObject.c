#include <math.h>
#include <stdio.h>
#include <string.h>

#define EMIBASE_INTERNAL

#include "EmiObject/EmiObject.h"
#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"
#include "EmiObject/LinkedObjectList.h"

void EObject_SetParent(void* object, EObject* parent)
{
    if(object == NULL)
        return;
    EObject* ctx = (EObject*)object;
    if(parent != NULL)
    {
        if(ctx->_ParentInternalTracking != NULL)
        {
            LinkedObjectList_remove(&ctx->_ParentInternalTracking->Children, ctx);
        } else {
            LinkedObjectList_remove(&root_objects, ctx);
        }
        ctx->Parent = parent;
        LinkedObjectList_append(&parent->Children, ctx);
    } else {
        if(ctx->_ParentInternalTracking != NULL)
        {
            LinkedObjectList_remove(&ctx->_ParentInternalTracking->Children, ctx);
            ctx->Parent = NULL;
            LinkedObjectList_append(&root_objects, ctx);
        }
    }
    ctx->_ParentInternalTracking = ctx->Parent;
}

extern void _emiobject_internal_wipe_recursively(LinkedObjectList* collection, EObject* object);
void EObject_Destroy(void* object)
{
    if(object == NULL)
        return;
    EObject* ctx = (EObject*)object;
    _emiobject_internal_wipe_recursively(&ctx->Children, ctx);
}

void EObject_SetName(void* object, const char* name)
{
    if(object == NULL)
        return;
    EObject* ctx = (EObject*)object;
    if(name == NULL)
    {
        if(ctx->Name != NULL)
            MemFree(ctx->Name);
        ctx->Name = NULL;
        return;
    }
    size_t len = strlen(name);
    if(len > 255) // Serialization limits
        return;
    if(len == 0)
    {
        if(ctx->Name != NULL)
            MemFree(ctx->Name);
        ctx->Name = NULL;
        return;
    }
    char* newname = MemAlloc(len + 1);
    if(!newname)
        return;
    strncpy(newname, name, len);
    if(ctx->Name != NULL)
        MemFree(ctx->Name);
    ctx->Name = newname;
}

void _eobject_internal_render(EObject* ctx, ETransform* parent, ETransform* out)
{
    if(!ctx->Visible || ctx->_render == NULL)
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

    localX -= parent->Size.x * 0.5f;
    localY -= parent->Size.y * 0.5f;

    localX += out->Size.x * (0.5f - ctx->Anchor.x);
    localY += out->Size.y * (0.5f - ctx->Anchor.y);

    float s = sinf(SDEG2RAD(parent->Rotation));
    float c = cosf(SDEG2RAD(parent->Rotation));

    out->Position.x = parent->Position.x + (localX * c - localY * s);
    out->Position.y = parent->Position.y + (localX * s + localY * c);

    out->Rotation = parent->Rotation + ctx->Rotation;

    ctx->_render(ctx, out);
}

#ifndef RELEASE
    void _eobject_internal_serialize(BufferWriter* writer, EObject* self)
    {
        if(self->Name != NULL)
        {
            uint8_t namelen = (uint8_t)strlen(self->Name);
            BW_WriteU8(writer, namelen);
            BW_WriteString(writer, self->Name, namelen);
        } else {
            BW_WriteU8(writer, 0);
        }
        EUDim2_serialize(writer, self->Position);
        EUDim2_serialize(writer, self->Size);
        BW_WriteFloat(writer, self->Rotation);
        Vector2_serialize(writer, self->Anchor);
        BW_WriteU8(writer, self->Visible ? 1 : 0);
        BW_WriteU8(writer, self->ZIndex);
    }
#endif


void _eobject_internal_initialize(EObject* object)
{
    object->Name = NULL;
    object->Position = UDim2_new(0.0f, 100, 0.0f, 100);
    if(object->innerType == EObjectType_EText) {
        object->Size = UDim2_new(0.0f, 200, 0.0f, 50);
    } else {
        object->Size = UDim2_new(0.0f, 100, 0.0f, 100);
    }
    object->Anchor = Vector2_new(0.0f, 0.0f);
    object->Rotation = 0.0f;
    object->Parent = NULL;
    object->_ParentInternalTracking = NULL;
    object->Children = LinkedObjectList_create();
    object->Visible = true;
    object->ZIndex = 0;

#ifndef RELEASE
    object->_nk_expanded = false;
#endif

    LinkedObjectList_append(&root_objects, object);
}