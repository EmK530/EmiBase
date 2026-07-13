#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    unsigned char* data;
    size_t size;
    size_t offset;
    int ownsMemory;
    int error;
} BufferWriter;

BufferWriter* BW_CreateWithCapacity(size_t capacity);
BufferWriter* BW_CreateFromMemory(unsigned char* data, size_t size);

void BW_Destroy(BufferWriter* bw);

int    BW_HasError(BufferWriter* bw);
size_t BW_GetSize(BufferWriter* bw);

const unsigned char* BW_GetData(BufferWriter* bw, size_t* outSize);

int BW_SaveToFile(BufferWriter* bw, const char* path);

void BW_WriteU8(BufferWriter* bw, uint8_t value);
void BW_WriteU16(BufferWriter* bw, uint16_t value);
void BW_WriteU32(BufferWriter* bw, uint32_t value);
void BW_WriteI32(BufferWriter* bw, int32_t value);
void BW_WriteVarU32(BufferWriter* bw, uint32_t value);
void BW_WriteFloat(BufferWriter* bw, float value);

void BW_WriteString(BufferWriter* bw, const char* str, size_t bytes);

void BW_Seek(BufferWriter* bw, size_t offset);
