#include "Libraries/BufferWriter.h"

#include <string.h>
#include "raylib.h"

static int BW_EnsureCapacity(BufferWriter* bw, size_t extra)
{
    if (!bw || bw->error) return 0;
    size_t required = bw->offset + extra;
    if (required <= bw->size) return 1;
    if (!bw->ownsMemory)
    {
        bw->error = 1;
        return 0;
    }
    size_t newSize = bw->size ? bw->size : 64;
    while (newSize < required) newSize *= 2;
    unsigned char* newData = (unsigned char*)MemAlloc((int)newSize);
    if (!newData)
    {
        bw->error = 1;
        return 0;
    }
    if (bw->data)
    {
        memcpy(newData, bw->data, bw->size);
        MemFree(bw->data);
    }
    bw->data = newData;
    bw->size = newSize;
    return 1;
}

static BufferWriter* BW_Alloc(void)
{
    BufferWriter* bw = (BufferWriter*)MemAlloc(sizeof(BufferWriter));
    if (!bw) return NULL;
    memset(bw, 0, sizeof(BufferWriter));
    return bw;
}

BufferWriter* BW_CreateWithCapacity(size_t capacity)
{
    BufferWriter* bw = BW_Alloc();
    if (!bw) return NULL;
    bw->ownsMemory = 1;
    if (capacity > 0)
    {
        bw->data = (unsigned char*)MemAlloc((int)capacity);
        if (!bw->data)
            return NULL;
        bw->size = capacity;
    }
    return bw;
}

BufferWriter* BW_CreateFromMemory(unsigned char* data, size_t size)
{
    BufferWriter* bw = BW_Alloc();
    if (!bw) return NULL;
    bw->data       = data;
    bw->size       = size;
    bw->ownsMemory = 0;
    return bw;
}

void BW_Destroy(BufferWriter* bw)
{
    if (!bw) return;
    if (bw->ownsMemory && bw->data)
        MemFree(bw->data);
    MemFree(bw);
}

int BW_HasError(BufferWriter* bw)
{
    return bw ? bw->error : 1;
}

size_t BW_GetSize(BufferWriter* bw)
{
    return bw ? bw->offset : 0;
}

const unsigned char* BW_GetData(BufferWriter* bw, size_t* outSize)
{
    if (!bw || bw->error)
    {
        if (outSize) *outSize = 0;
        return NULL;
    }
    if (outSize) *outSize = bw->offset;
    return bw->data;
}

int BW_SaveToFile(BufferWriter* bw, const char* path)
{
    if (!bw || bw->error || !bw->data || !path) return 0;
    return SaveFileData(path, bw->data, (int)bw->offset);
}

void BW_WriteU8(BufferWriter* bw, uint8_t value)
{
    if (!BW_EnsureCapacity(bw, 1)) return;
    bw->data[bw->offset++] = value;
}

void BW_WriteU16(BufferWriter* bw, uint16_t value)
{
    if (!BW_EnsureCapacity(bw, 2)) return;
    memcpy(bw->data + bw->offset, &value, 2);
    bw->offset += 2;
}

void BW_WriteU32(BufferWriter* bw, uint32_t value)
{
    if (!BW_EnsureCapacity(bw, 4)) return;
    memcpy(bw->data + bw->offset, &value, 4);
    bw->offset += 4;
}

void BW_WriteI32(BufferWriter* bw, int32_t value)
{
    if (!BW_EnsureCapacity(bw, 4)) return;
    memcpy(bw->data + bw->offset, &value, 4);
    bw->offset += 4;
}

void BW_WriteVarU32(BufferWriter* bw, uint32_t value)
{
    do
    {
        uint8_t byte = value & 0x7F;
        value >>= 7;
        if (value != 0) byte |= 0x80;
        if (!BW_EnsureCapacity(bw, 1)) return;
        bw->data[bw->offset++] = byte;
    } while (value != 0);
}

void BW_WriteFloat(BufferWriter* bw, float value)
{
    if (!BW_EnsureCapacity(bw, 4)) return;
    memcpy(bw->data + bw->offset, &value, 4);
    bw->offset += 4;
}

void BW_WriteString(BufferWriter* bw, const char* str, size_t bytes)
{
    if (!BW_EnsureCapacity(bw, bytes)) return;
    memcpy(bw->data + bw->offset, str, bytes);
    bw->offset += bytes;
}

void BW_Seek(BufferWriter* bw, size_t offset)
{
    if (!bw) return;
    if (offset > bw->size)
    {
        bw->error = 1;
        return;
    }
    bw->offset = offset;
}
