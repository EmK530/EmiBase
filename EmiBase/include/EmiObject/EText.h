#pragma once

#include "EmiObject/EObject.h"
#include "EmiObject/Types.h"

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"
#endif

#ifndef ETEXT_VERSION
    #define ETEXT_VERSION 1
#endif

#define ETEXT_TEXTURE_PADDING 1

enum ETextRenderType
{
    ETextRenderType_Fixed = 1, // The text is drawn with its font size directly.
    ETextRenderType_FixedFit = 2, // The text is drawn with its font size and scaled to fit its container.
    ETextRenderType_FixedStretch = 3, // The text is drawn with its font size and scaled to stretch into its container.
    //ETextRenderType_Auto = 4, // The text is drawn to fit its container with an automatically determined font size.
    // TODO: Maybe implement AutoWrap in the future?
};

#ifdef EMIBASE_INTERNAL
    typedef struct
    {
        EOBJECT_BASE_TYPES
        Color BackgroundColor;
        Color TextColor;
        const char* Text;
        RenderTexture2D _CachedDraw;
        Font _CachedFont;
        const char* _FontPath;
        size_t _TextTrueLen;
        size_t _TextBufferLen;
        float _FontSize;
        uint8_t _RenderType;
        bool _dirty;
        bool _isFontValid;
        bool _isTextureValid;
    } EText;
#else
    typedef struct
    {
        EOBJECT_BASE_TYPES
        Color BackgroundColor;
        Color TextColor;
        const char* Text;
        uint8_t _reserved[sizeof(RenderTexture2D) + sizeof(Font) + sizeof(const char*) + sizeof(size_t)*2 + sizeof(float) + sizeof(uint8_t) + sizeof(bool)*3];
    } EText;
#endif

void EText_SetText(EText* target, const char* text);
void EText_SetTextN(EText* target, const char* text, size_t len);
void EText_SetRenderType(EText* target, enum ETextRenderType type);
void EText_SetFont(EText* target, const char* fontPath);
void EText_SetFontSize(EText* target, float fontSize);
EText* EText_Create(EObject* parent);