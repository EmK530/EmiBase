#pragma once

#include <stdint.h>

#include "raylib.h"

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
    int32_t Offset;
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

#include "Libraries/BufferReader.h"
static inline Color Color32_deserialize(BufferReader* reader)
{
    Color c = {
        BR_ReadU8(reader),
        BR_ReadU8(reader),
        BR_ReadU8(reader),
        BR_ReadU8(reader)
    };
    return c;
}

static inline EUDim2 EUDim2_deserialize(BufferReader* reader)
{
    EUDim e = { BR_ReadFloat(reader), BR_ReadI32(reader) };
    EUDim f = { BR_ReadFloat(reader), BR_ReadI32(reader) };
    EUDim2 u = { e, f };
    return u;
}

static inline Vector2 Vector2_deserialize(BufferReader* reader)
{
    Vector2 e = { BR_ReadFloat(reader), BR_ReadFloat(reader) };
    return e;
}

#ifndef RELEASE
    #include "Libraries/BufferWriter.h"

    static inline void Color32_serialize(BufferWriter* writer, Color input)
    {
        BW_WriteU8(writer, input.r);
        BW_WriteU8(writer, input.g);
        BW_WriteU8(writer, input.b);
        BW_WriteU8(writer, input.a);
    }

    static inline void EUDim2_serialize(BufferWriter* writer, EUDim2 input)
    {
        BW_WriteFloat(writer, input.X.Scale);
        BW_WriteI32(writer, input.X.Offset);
        BW_WriteFloat(writer, input.Y.Scale);
        BW_WriteI32(writer, input.Y.Offset);
    }

    static inline void Vector2_serialize(BufferWriter* writer, Vector2 input)
    {
        BW_WriteFloat(writer, input.x);
        BW_WriteFloat(writer, input.y);
    }
#endif