#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "EmiBase.h"

#define SPAK_MAGIC 0x4B415045 // "EPAK"
#define SFCH_MAGIC 0x48434645 // "EFCH"

static FILE *gPakFile = NULL;
static LinkedList *gEntries = NULL;

// These get modified at runtime
static bool isEncrypted = false;
static bool hasCRC = false;

static uint32_t xorshift32(uint32_t state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

static void crypt_buffer(unsigned char *buffer, size_t size, uint32_t fileOffset)
{
    uint32_t state = CONTENT_KEY ^ fileOffset;
    for(int i = 0; i < 8; i++)
        state = xorshift32(state);
    for(size_t i = 0; i < size; i += 4)
    {
        state = xorshift32(state);
        uint8_t *key = (uint8_t*)&state;
        if(i + 0 < size) buffer[i + 0] ^= key[0];
        if(i + 1 < size) buffer[i + 1] ^= key[1];
        if(i + 2 < size) buffer[i + 2] ^= key[2];
        if(i + 3 < size) buffer[i + 3] ^= key[3];
    }
}

static int read(void *buffer, size_t size)
{
    int result = fread(buffer, 1, size, gPakFile);
    if(isEncrypted)
    {
        long offset = ftell(gPakFile);
        crypt_buffer((unsigned char*)buffer, size, (uint32_t)offset);
    }
    return result;
}

static ContentEntry *find_entry(const char *path)
{
    LinkedList_foreach(gEntries, node)
    {
        ContentEntry *entry = node->item;
        if(strcmp(entry->path, path) == 0)
            return entry;
    }
    return NULL;
}

static void free_entry(void *ptr)
{
    ContentEntry *entry = ptr;
    MemFree(entry->path);
    MemFree(entry);
}

static uint32_t crc32_table[256];

static void crc32_init(void)
{
    for(uint32_t i = 0; i < 256; i++)
    {
        uint32_t crc = i;

        for(uint32_t j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }

        crc32_table[i] = crc;
    }
}

static uint32_t crc32_compute(const void *data, size_t length)
{
    const uint8_t *bytes = (const uint8_t*)data;

    uint32_t crc = 0xFFFFFFFF;

    for(size_t i = 0; i < length; i++)
    {
        uint8_t index = (uint8_t)((crc ^ bytes[i]) & 0xFF);
        crc = (crc >> 8) ^ crc32_table[index];
    }

    return ~crc;
}

int ContentManager_Init(const char *pakPath)
{
    crc32_init();
    gPakFile = fopen(pakPath, "rb");

    if(!gPakFile)
    {
        eprintf("[ContentManager] Failed to open pak file\n");
        WinMessageBox("Fatal error!", "EmiBase failed to launch.\n\nCould not open the game content file, is it missing?", MB_TOPMOST | MB_ICONERROR);
        return 0;
    }

    gEntries = LinkedList_create();

    uint32_t magic;
    read(&magic, 4);

    if(magic != SPAK_MAGIC)
    {
        eprintf("[ContentManager] Invalid pak header\n");
        WinMessageBox("Fatal error!", "EmiBase failed to launch.\n\nGame content is corrupted. (ERR 1)", MB_TOPMOST | MB_ICONERROR);

        fclose(gPakFile);
        gPakFile = NULL;

        return 0;
    }

    uint8_t version;
    read(&version, 1);

    if(version != 2)
    {
        eprintf("[ContentManager] Invalid pak version\n");
        WinMessageBox("Fatal error!", "EmiBase failed to launch.\n\nUnsupported game content version.", MB_TOPMOST | MB_ICONERROR);
        return 0;
    }

    uint8_t flags;
    read(&flags, 1);

    isEncrypted = (flags & 1) != 0;
    hasCRC = (flags & 2) != 0;

    uint32_t fileCount;
    read(&fileCount, 4);

    for(uint32_t i = 0; i < fileCount; i++)
    {
        uint32_t chunkMagic;
        read(&chunkMagic, 4);

        if(chunkMagic != SFCH_MAGIC)
        {
            eprintf("[ContentManager] Invalid chunk header\n");
            WinMessageBox("Fatal error!", "EmiBase failed to launch.\n\nGame content is corrupted. (ERR 2)", MB_TOPMOST | MB_ICONERROR);
            return 0;
        }

        uint8_t pathLength;
        uint32_t fileSize;
        uint32_t storedPathCRC = 0;
        uint32_t storedDataCRC = 0;

        read(&pathLength, 1);
        read(&fileSize, 4);
        if(hasCRC)
        {
            read(&storedPathCRC, 4);
            read(&storedDataCRC, 4);
        }

        char *path = MemAlloc(pathLength + 1);

        read(path, pathLength);

        path[pathLength] = '\0';

        if(hasCRC)
        {
            uint32_t actualPathCRC = crc32_compute(path, pathLength);
            if(actualPathCRC != storedPathCRC)
            {
                eprintf("[ContentManager] Path CRC mismatch\n");
                WinMessageBox("Fatal error!", "EmiBase failed to launch.\n\nGame content is corrupted. (ERR 3)", MB_TOPMOST | MB_ICONERROR);
                MemFree(path);
                fclose(gPakFile);
                gPakFile = NULL;
                return 0;
            }
        }

        ContentEntry *entry = MemAlloc(sizeof(ContentEntry));

        entry->path = path;
        entry->dataOffset = (uint32_t)ftell(gPakFile);
        entry->size = fileSize;
        entry->pathCRC = storedPathCRC;
        entry->dataCRC = storedDataCRC;

        fseek(gPakFile, fileSize, SEEK_CUR);

        LinkedList_append(gEntries, entry);
    }

    eprintf("[ContentManager] Loaded %i pak entries\n", gEntries->size);

    return 1;
}

void ContentManager_Dispose()
{
    if(gPakFile)
    {
        fclose(gPakFile);
        gPakFile = NULL;
    }
    if(gEntries)
        LinkedList_dispose(&gEntries, free_entry);
}

unsigned char *ContentManager_LoadFile(const char *path, size_t *size)
{
    ContentEntry *entry = find_entry(path);
    if(!entry)
    {
        eprintf("[ContentManager] Missing pak asset: %s\n", path);
        return NULL;
    }
    unsigned char *data = MemAlloc(entry->size);
    if(!data)
    {
        eprintf("[ContentManager] Out of memory loading asset: %s\n", path);

        char msg[67];
        snprintf(msg, 67, "Out of memory, ContentManager failed to allocate %i bytes.", entry->size);
        WinMessageBox("Fatal error!", msg, MB_TOPMOST | MB_ICONERROR);
        CloseWindow();
        WinExitProcess(1);
        
        return NULL;
    }
    fseek(gPakFile, entry->dataOffset, SEEK_SET);
    read(data, entry->size);
    if(hasCRC)
    {
        uint32_t actualCRC = crc32_compute(data, entry->size);
        if(actualCRC != entry->dataCRC)
        {
            eprintf("[ContentManager] CRC mismatch for asset '%s' (expected %08X got %08X)\n", path, entry->dataCRC, actualCRC);
            WinMessageBox("Fatal error!", "EmiBase detected corrupted game content at runtime.\n\nPlease verify the game files.", MB_TOPMOST | MB_ICONERROR);
            MemFree(data);
            CloseWindow();
            WinExitProcess(1);
            return NULL;
        }
    }
    if(size)
        *size = entry->size;
    return data;
}

Texture2D ContentManager_LoadTexture(const char *path)
{
    size_t size;
    unsigned char *data = ContentManager_LoadFile(path, &size);
    if(!data)
        return (Texture2D){0};
    Image image = LoadImageFromMemory(GetFileExtension(path), data, size);
    MemFree(data);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

Image ContentManager_LoadImage(const char *path)
{
    size_t size;
    unsigned char *data = ContentManager_LoadFile(path, &size);
    if(!data)
        return (Image){0};
    Image image = LoadImageFromMemory(GetFileExtension(path), data, size);
    MemFree(data);
    return image;
}

Font ContentManager_LoadFont(const char *path, int fontSize)
{
    size_t size;
    unsigned char *data = ContentManager_LoadFile(path, &size);
    if(!data)
        return GetFontDefault();
    Font font = LoadFontFromMemory(".ttf", data, size, fontSize, NULL, 0);
    MemFree(data);
    return font;
}

Shader ContentManager_LoadShader(const char *vsPath, const char *fsPath)
{
    char *vsCode = vsPath ? ContentManager_LoadText(vsPath) : NULL;
    char *fsCode = fsPath ? ContentManager_LoadText(fsPath) : NULL;
    Shader shader = LoadShaderFromMemory(vsCode, fsCode);
    if(vsCode) MemFree(vsCode);
    if(fsCode) MemFree(fsCode);
    return shader;
}

Music ContentManager_LoadMusic(const char *path)
{
    size_t size;
    unsigned char *data = ContentManager_LoadFile(path, &size);
    if(!data)
        return (Music){0};
    return LoadMusicStreamFromMemory(GetFileExtension(path), data, size);
}

Sound ContentManager_LoadSound(const char *path)
{
    size_t size;
    unsigned char *data = ContentManager_LoadFile(path, &size);
    if(!data)
        return (Sound){0};
    Wave wave = LoadWaveFromMemory(GetFileExtension(path), data, size);
    MemFree(data);
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

char *ContentManager_LoadText(const char *path)
{
    size_t size;
    unsigned char *data = ContentManager_LoadFile(path, &size);
    if(!data)
        return NULL;
    char *text = MemAlloc(size + 1);
    memcpy(text, data, size);
    text[size] = '\0';
    MemFree(data);
    return text;
}