#include "EmiBase/FontManager.h"
#include "EmiBase/ContentManager.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    char* key; // "path#size"
    Font font;
} FontAsset;

typedef struct {
    FontAsset* items;
    int count;
    int capacity;
} FontCache;

static FontCache cache = {0};
static char* staticPath = "font/";

void FontManager_Init(void)
{
    // Nothing needed for now
}

void FontManager_Shutdown(void)
{
    for (int i = 0; i < cache.count; i++)
    {
        UnloadFont(cache.items[i].font);
        MemFree(cache.items[i].key);
    }

    MemFree(cache.items);
    cache.items = NULL;
    cache.count = 0;
    cache.capacity = 0;
}

static void MakeKey(char* out, const char* path, int size)
{
    sprintf(out, "%s#%d", path, size);
}

static FontAsset* Find(const char* key)
{
    for (int i = 0; i < cache.count; i++)
    {
        if (strcmp(cache.items[i].key, key) == 0)
            return &cache.items[i];
    }
    return NULL;
}

Font* FontManager_GetFont(const char* path, int size)
{
    int size2 = strlen(path) + strlen(staticPath) + 1;
    char* finalPath = emalloc_strict(size2);
    snprintf(finalPath, size2, "%s%s", staticPath, path);

    char key[512];
    MakeKey(key, finalPath, size);

    FontAsset* found = Find(key);
    if (found)
    {
        MemFree(finalPath);
        return &found->font;
    }

    if (cache.count >= cache.capacity)
    {
        cache.capacity = cache.capacity ? cache.capacity * 2 : 8;
        cache.items = MemRealloc(cache.items, sizeof(FontAsset) * cache.capacity);
    }

    FontAsset* a = &cache.items[cache.count++];

    a->key = _strdup(key);
    a->font = ContentManager_LoadFont(finalPath, size);

    MemFree(finalPath);

    return &a->font;
}

void FontManager_DrawText(const char* path, int size, const char* text, Vector2 position, Color color)
{
    Font* font = FontManager_GetFont(path, size);

    DrawTextEx(*font, text, position, (float)size, 0, color);
}