#pragma once

#include <stdint.h>

#include "EmiBase.h"

// Color32

static inline Color Color32_new(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    Color c = {r, g, b, a};
    return c;
}

// Rectangle

static inline Rectangle Rectangle_new(float a, float b)
{
    Rectangle e = { a, b };
    return e;
}

// UDim & UDim2

typedef struct
{
    float Scale;
    uint32_t Offset;
} EUDim;

typedef struct
{
    EUDim X;
    EUDim Y;
} EUDim2;

static inline EUDim UDim_new(float a, uint32_t b)
{
    EUDim e = { a, b };
    return e;
}

static inline EUDim2 UDim2_new(float a, uint32_t b, float c, uint32_t d)
{
    EUDim e = { a, b };
    EUDim f = { c, d };
    EUDim2 u = { e, f };
    return u;
}

static inline EUDim2 UDim2_fromScale(float a, float c)
{
    EUDim e = { a, 0 };
    EUDim f = { c, 0 };
    EUDim2 u = { e, f };
    return u;
}

static inline EUDim2 UDim2_fromOffset(uint32_t b, uint32_t d)
{
    EUDim e = { 0.0, b };
    EUDim f = { 0.0, d };
    EUDim2 u = { e, f };
    return u;
}

// Vector2 & Vector2i

static inline Vector2 Vector2_new(float a, float b)
{
    Vector2 e = { a, b };
    return e;
}

typedef struct
{
    int X;
    int Y;
} EVector2i;

static inline EVector2i Vector2i_new(int a, int b)
{
    EVector2i e = { a, b };
    return e;
}