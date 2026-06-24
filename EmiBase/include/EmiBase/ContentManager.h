#ifndef CONTENT_MANAGER_H
#define CONTENT_MANAGER_H

#include <stdint.h>
#include <stdio.h>

#include "Libraries/LinkedList.h"

typedef struct
{
    char *path;

    uint32_t dataOffset;
    uint32_t size;

    uint32_t pathCRC;
    uint32_t dataCRC;
} ContentEntry;

int ContentManager_Init(const char *pakPath);
void ContentManager_Dispose();

unsigned char *ContentManager_LoadFile(
    const char *path,
    size_t *size);

Texture2D ContentManager_LoadTexture(const char *path);

Image ContentManager_LoadImage(const char *path);

Font ContentManager_LoadFont(const char *path, int fontSize);

Shader ContentManager_LoadShader(const char *vsPath, const char *fsPath);

Music ContentManager_LoadMusic(const char *path);

Sound ContentManager_LoadSound(const char *path);

char *ContentManager_LoadText(const char *path);

#endif