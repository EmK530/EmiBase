#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    unsigned char* data;
    size_t size;
    size_t offset;
    int ownsMemory;
    int error;
} BufferReader;

BufferReader* BR_CreateFromMemory(const unsigned char* data, size_t size);
BufferReader* BR_CreateFromFile(const char* path);

void BR_Destroy(BufferReader* br);

int BR_HasError(BufferReader* br);

uint8_t  BR_ReadU8(BufferReader* br);
uint16_t BR_ReadU16(BufferReader* br);
uint32_t BR_ReadU32(BufferReader* br);
int32_t  BR_ReadI32(BufferReader* br);
uint32_t BR_ReadVarU32(BufferReader* br);
float    BR_ReadFloat(BufferReader* br);

char* BR_ReadString(BufferReader* br, size_t bytes);

void BR_Skip(BufferReader* br, size_t bytes);
void BR_Seek(BufferReader* br, size_t offset);