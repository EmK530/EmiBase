#include "Libraries/BufferReader.h"

#include <string.h>
#include "raylib.h"

static int BR_CanRead(BufferReader* br, size_t bytes)
{
    if (!br || br->offset + bytes > br->size)
    {
        if (br) br->error = 1;
        return 0;
    }
    return 1;
}

static BufferReader* BR_Alloc(void)
{
    BufferReader* br = (BufferReader*)MemAlloc(sizeof(BufferReader));
    if (!br) return NULL;

    memset(br, 0, sizeof(BufferReader));
    return br;
}

BufferReader* BR_CreateFromMemory(const unsigned char* data, size_t size)
{
    BufferReader* br = BR_Alloc();
    if (!br) return NULL;

    br->data = (unsigned char*)data;
    br->size = size;
    br->ownsMemory = 0;

    return br;
}

BufferReader* BR_CreateFromFile(const char* path)
{
    BufferReader* br = BR_Alloc();
    if (!br) return NULL;

    unsigned int sz = 0;
    unsigned char* data = LoadFileData(path, &sz);

    if (!data)
    {
        MemFree(br);
        return NULL;
    }

    br->data = data;
    br->size = sz;
    br->ownsMemory = 1;

    return br;
}

void BR_Destroy(BufferReader* br)
{
    if (!br) return;

    if (br->ownsMemory && br->data)
        UnloadFileData(br->data);

    MemFree(br);
}

int BR_HasError(BufferReader* br)
{
    return br ? br->error : 1;
}

// ---- Reads ----

uint8_t BR_ReadU8(BufferReader* br)
{
    if (!BR_CanRead(br, 1)) return 0;
    return br->data[br->offset++];
}

uint32_t BR_ReadU32(BufferReader* br)
{
    if (!BR_CanRead(br, 4)) return 0;

    uint32_t v;
    memcpy(&v, br->data + br->offset, 4);
    br->offset += 4;
    return v;
}

int32_t BR_ReadI32(BufferReader* br)
{
    if (!BR_CanRead(br, 4)) return 0;

    int32_t v;
    memcpy(&v, br->data + br->offset, 4);
    br->offset += 4;
    return v;
}

uint32_t BR_ReadVarU32(BufferReader* br)
{
    uint32_t result = 0;
    uint32_t shift = 0;
    while (1)
    {
        if (!BR_CanRead(br, 1))
            return 0;
        uint8_t byte = br->data[br->offset++];
        result |= (uint32_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            break;
        shift += 7;
        // Prevent overflow / malformed data
        if (shift >= 32)
        {
            br->error = 1;
            return 0;
        }
    }
    return result;
}

float BR_ReadFloat(BufferReader* br)
{
    if (!BR_CanRead(br, 4)) return 0.0f;

    float v;
    memcpy(&v, br->data + br->offset, 4);
    br->offset += 4;
    return v;
}

// ---- String ----

char* BR_ReadString(BufferReader* br, size_t bytes)
{
    if (!BR_CanRead(br, bytes)) return NULL;
    char* response = MemAlloc(bytes + 1);
    if(!response) return NULL;
    response[bytes] = '\0';
    memcpy(response, br->data + br->offset, bytes);
    br->offset += bytes;
    return response;
}

// ---- Navigation ----

void BR_Skip(BufferReader* br, size_t bytes)
{
    if (BR_CanRead(br, bytes))
        br->offset += bytes;
}

void BR_Seek(BufferReader* br, size_t offset)
{
    if (!br || offset > br->size)
    {
        if (br) br->error = 1;
        return;
    }

    br->offset = offset;
}