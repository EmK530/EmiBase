#include <stdio.h>
#include <string.h>

#define EMIBASE_INTERNAL

#include "EmiBase.h"

void _etext_internal_render(EText* text, ETransform* t)
{
    if(text->BackgroundColor.a > 1)
    {
        if (t->Rotation == 0.0f) {
            DrawRectangle((int)(t->Position.x - t->Size.x * 0.5f),(int)(t->Position.y - t->Size.y * 0.5f),(int)t->Size.x,(int)t->Size.y,text->BackgroundColor);
        } else {
            Rectangle r = { t->Position.x, t->Position.y, t->Size.x, t->Size.y };
            Vector2 origin = { t->Size.x * 0.5f, t->Size.y * 0.5f };
            DrawRectanglePro(r, origin, t->Rotation, text->BackgroundColor);
        }
    }

    if(text->_dirty && text->_isFontValid)
    {
        switch(text->_RenderType)
        {
            case ETextRenderType_Fixed:
            case ETextRenderType_FixedFit:
            case ETextRenderType_FixedStretch:
            {
                if(text->_isTextureValid)
                    UnloadRenderTexture(text->_CachedDraw);
                Vector2 bounds = MeasureTextEx(text->_CachedFont, text->Text, text->_FontSize, 0);
                text->_CachedDraw = EmiBase_LoadRenderTexture((int)ceilf(bounds.x)+ETEXT_TEXTURE_PADDING, (int)ceilf(bounds.y));
                text->_isTextureValid = IsRenderTextureValid(text->_CachedDraw);
                if(text->_isTextureValid)
                {
                    EmiBase_Detach();
                    EmiBase_DetachedTextureMode(text->_CachedDraw);
                    DrawTextEx(text->_CachedFont, text->Text, Vector2_new(0,0), text->_FontSize, 0, WHITE);
                    EmiBase_Attach();
                    text->_dirty = false;
                }
                break;
            }
            //case ETextRenderType_Auto:
            {
                // TODO
                break;
            }
        }
    }

    switch(text->_RenderType)
    {
        case ETextRenderType_Fixed:
        {
            if(text->_isTextureValid)
            {
                int w = text->_CachedDraw.texture.width; int h = text->_CachedDraw.texture.height;
                Rectangle src = { 0, 0, w, -h };
                Rectangle r = { t->Position.x, t->Position.y, w, h };
                Vector2 origin = { w * 0.5f, h * 0.5f };
                DrawTexturePro(text->_CachedDraw.texture, src, r, origin, t->Rotation, text->TextColor);
            }
            break;
        }
        case ETextRenderType_FixedFit:
        {
            if(text->_isTextureValid)
            {
                int w = text->_CachedDraw.texture.width;
                int h = text->_CachedDraw.texture.height;
                float scale = fminf(t->Size.x / (float)w, t->Size.y / (float)h);
                float dw = w * scale;
                float dh = h * scale;
                Rectangle src = { 0, 0, w, -h };
                Rectangle r = { t->Position.x, t->Position.y, dw, dh };
                Vector2 origin = { dw * 0.5f, dh * 0.5f };
                DrawTexturePro(text->_CachedDraw.texture, src, r, origin, t->Rotation, text->TextColor);
            }
            break;
        }
        case ETextRenderType_FixedStretch:
        {
            if(text->_isTextureValid)
            {
                int w = text->_CachedDraw.texture.width; int h = text->_CachedDraw.texture.height;
                Rectangle src = { 0, 0, w, -h };
                Rectangle r = { t->Position.x, t->Position.y, t->Size.x, t->Size.y };
                Vector2 origin = { t->Size.x * 0.5f, t->Size.y * 0.5f };
                DrawTexturePro(text->_CachedDraw.texture, src, r, origin, t->Rotation, text->TextColor);
            }
            break;
        }
    }
}

#ifndef RELEASE
    void _etext_internal_serialize(BufferWriter* writer, EText* self)
    {
        BW_WriteU8(writer, EObjectType_EText); // Type identifier
        Color32_serialize(writer, self->BackgroundColor);
        Color32_serialize(writer, self->TextColor);
        BW_WriteU32(writer, self->_TextTrueLen);
        BW_WriteString(writer, self->Text, self->_TextTrueLen);
        uint8_t fontPathLen = strlen(self->_FontPath);
        BW_WriteU8(writer, fontPathLen);
        BW_WriteString(writer, self->_FontPath, fontPathLen);
        BW_WriteFloat(writer, self->_FontSize);
        BW_WriteU8(writer, self->_RenderType);
    }
#endif

extern void _eobject_internal_initialize(EObject* object);

void _etext_internal_free(EObject* item)
{
    EText* text = (EText*)item;
    if(text->Text != NULL)
        MemFree((void*)text->Text);
    if(text->_FontPath != NULL)
        MemFree((void*)text->_FontPath);
    if(text->_isTextureValid)
        UnloadRenderTexture(text->_CachedDraw);
    if(text->_isFontValid)
        UnloadFont(text->_CachedFont);
}

uint32_t next_pow2(uint32_t n)
{
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

static char message[128];

void AllocationError(uint32_t byteAmount)
{
    snprintf(message, 128, "Ran out of memory or heap is fragmented.\n\nEmiBase failed to allocate %i bytes for an EText object.", byteAmount);
    WinMessageBox("Fatal error!", message, MB_TOPMOST | MB_ICONERROR);
    CloseWindow();
    WinExitProcess(1);
}

// Update the Text property of an EText object, requires a string length parameter.
void EText_SetTextN(EText* target, const char* text, size_t len)
{
    len++;
    if(target->Text != NULL)
    {
        if(len > target->_TextBufferLen)
        {
            uint32_t bufSizeNew = next_pow2(len);
            char* newBuf = MemRealloc((void*)target->Text, bufSizeNew);
            if(newBuf == NULL)
                AllocationError(bufSizeNew);
            target->Text = newBuf;
            target->_TextBufferLen = bufSizeNew;
        } else {
            if(target->_TextTrueLen == len-1 && strncmp(target->Text, text, len-1) == 0)
                return;
        }
    } else {
        uint32_t bufSizeNew = next_pow2(len);
        const char* newBuf = MemAlloc(bufSizeNew);
        if(newBuf == NULL)
            AllocationError(bufSizeNew);
        target->Text = newBuf;
        target->_TextBufferLen = bufSizeNew;
    }
    target->_dirty = true;
    memcpy((void*)target->Text, text, --len);
    ((char*)target->Text)[len] = '\0';
    target->_TextTrueLen = len;
}

void EText_SetRenderType(EText* target, enum ETextRenderType type)
{
    if(type == target->_RenderType)
        return;
    //target->_dirty = false; // TODO: May have to flip this if Auto needs it in the future
    target->_RenderType = type;
    /*
    if(type == ETextRenderType_Auto && target->_isTextureValid)
    {
        target->_isTextureValid = false;
        UnloadRenderTexture(target->_CachedDraw);
    }
    */
}

// Update the Text property of an EText object, requires a null-terminated string.
void EText_SetText(EText* target, const char* text)
{
    EText_SetTextN(target, text, strlen(text));
}

void EText_SetFont(EText* target, const char* fontPath)
{
    size_t dataSize = strlen(fontPath) + 1;
    if(target->_FontPath != NULL && strcmp(target->_FontPath, fontPath) == 0)
        return;
#ifndef RELEASE
    if(target->_FontPath != NULL)
        MemFree((void*)target->_FontPath);
    target->_FontPath = MemAlloc(dataSize);
    if(target->_FontPath != NULL)
        memcpy((void*)target->_FontPath, fontPath, dataSize);
#endif
    if(target->_isFontValid)
    {
        target->_isFontValid = false;
        UnloadFont(target->_CachedFont);
    }
    target->_CachedFont = ContentManager_LoadFont(fontPath, target->_FontSize);
    if(!IsFontValid(target->_CachedFont))
        return;
    target->_dirty = true;
    target->_isFontValid = true;
}

void EText_SetFontSize(EText* target, float fontSize)
{
    if(target->_isFontValid)
    {
        target->_isFontValid = false;
        UnloadFont(target->_CachedFont);
    }
    target->_FontSize = fontSize;
    target->_CachedFont = ContentManager_LoadFont(target->_FontPath, target->_FontSize);
    if(!IsFontValid(target->_CachedFont))
        return;
    target->_dirty = true;
    target->_isFontValid = true;
}

/*
    Creates a new Text EmiObject.
    Set parent to NULL to create as a root object.
*/
EText* EText_Create(EObject* parent)
{
    EText* text = (EText*)MemAlloc(sizeof(EText));
    if(!text)
    {
        eprintf("Out of memory allocating EText\n");
        return NULL;
    }

    text->innerType = EObjectType_EText;
    _eobject_internal_initialize((EObject*)text);

    text->_free_func = _etext_internal_free;
    text->_render = (void(*)(EObject*, ETransform*))_etext_internal_render;
#ifndef RELEASE
    text->_serialize_func = (void(*)(BufferWriter*, EObject*))_etext_internal_serialize;
#endif
    EObject_SetName(text, "EText");

    text->BackgroundColor = Color32_new(255,255,255,255);
    text->TextColor = Color32_new(0,0,0,255);
    text->_TextTrueLen = 0;
    text->_TextBufferLen = 0;
    text->_dirty = true;
    text->_isFontValid = false;
    text->_isTextureValid = false;
    text->_FontSize = 24.0f;
    text->_RenderType = ETextRenderType_Fixed;
    EText_SetTextN(text, "EText", 5);
    EText_SetFont(text, "font/ArialCE.ttf");

    if(parent != NULL)
        EObject_SetParent(text, parent);

    return text;
}