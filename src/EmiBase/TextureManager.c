#include "EmiBase/TextureManager.h"
#include "EmiBase/ContentManager.h"

#include "raylib.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Internal cache
// ---------------------------------------------------------------------------

#define TEXTURE_CACHE_MAX 256

typedef struct {
    char            path[512];
    Texture2D       texture;
    TextureFilter filterMode;   // filter applied to this texture
    bool            occupied;
} TextureCacheEntry;

static TextureCacheEntry s_cache[TEXTURE_CACHE_MAX];
static bool              s_initialised = false;

static void Cache_Init(void)
{
    if (s_initialised) return;
    memset(s_cache, 0, sizeof(s_cache));
    s_initialised = true;
}

// Returns a matching cache entry, or NULL if not found.
static TextureCacheEntry *Cache_Find(const char *path, TextureFilter filterMode)
{
    for (int i = 0; i < TEXTURE_CACHE_MAX; i++) {
        if (s_cache[i].occupied
            && s_cache[i].filterMode == filterMode
            && strcmp(s_cache[i].path, path) == 0)
        {
            return &s_cache[i];
        }
    }
    return NULL;
}

// Stores a newly loaded texture. Returns the entry, or NULL if the cache is full.
static TextureCacheEntry *Cache_Store(const char *path,
                                      Texture2D texture,
                                      TextureFilter filterMode)
{
    for (int i = 0; i < TEXTURE_CACHE_MAX; i++) {
        if (!s_cache[i].occupied) {
            strncpy(s_cache[i].path, path, sizeof(s_cache[i].path) - 1);
            s_cache[i].path[sizeof(s_cache[i].path) - 1] = '\0';
            s_cache[i].texture    = texture;
            s_cache[i].filterMode = filterMode;
            s_cache[i].occupied   = true;
            return &s_cache[i];
        }
    }
    TraceLog(LOG_WARNING, "TEXTUREMGR: Cache is full (%d entries). '%s' will not be cached.",
             TEXTURE_CACHE_MAX, path);
    return NULL;
}

// Applies the requested filter to a texture.
static void ApplyFilter(Texture2D texture, TextureFilter filterMode)
{
    switch (filterMode) {
        case TEXTURE_FILTER_BILINEAR:
            SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
            break;
        case TEXTURE_FILTER_POINT:
        default:
            SetTextureFilter(texture, TEXTURE_FILTER_POINT);
            break;
    }
}

// Loads a texture (or returns a cached one), applying the correct filter.
static Texture2D GetTexture(const char *path, TextureFilter filterMode, char cmgr)
{
    Cache_Init();

    TextureCacheEntry *entry = Cache_Find(path, filterMode);
    if (entry) return entry->texture;

    // Not cached — load from disk.
    Texture2D texture;
    if(!cmgr) {
        texture = ContentManager_LoadTexture(path);
    } else {
        texture = LoadTexture(path);
    }
    if (texture.id == 0) {
        TraceLog(LOG_ERROR, "TEXTUREMGR: Failed to load texture '%s'", path);
        return texture;
    }

    ApplyFilter(texture, filterMode);
    Cache_Store(path, texture, filterMode);

    TraceLog(LOG_INFO, "TEXTUREMGR: Loaded and cached '%s' (filter=%s)",
             path, filterMode == TEXTURE_FILTER_BILINEAR ? "bilinear" : "point");

    return texture;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void TextureManager_DrawTexture(const char *path,
                                Vector2    position,
                                Vector2    size,
                                Color      tint,
                                TextureFilter filterMode,
                                char externalAsset)
{
    Texture2D tex = GetTexture(path, filterMode, externalAsset);
    if (tex.id == 0) return;

    // (Vector2){0,0} means "use native texture size"
    float drawW = (size.x > 0.0f) ? size.x : (float)tex.width;
    float drawH = (size.y > 0.0f) ? size.y : (float)tex.height;

    Rectangle sourceRect = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    Rectangle destRect   = { position.x, position.y, drawW, drawH };
    Vector2   origin     = { 0.0f, 0.0f };

    DrawTexturePro(tex, sourceRect, destRect, origin, 0.0f, tint);
}

void TextureManager_DrawTextureRect(const char *path,
                                    Rectangle  sourceRect,
                                    Vector2    position,
                                    Vector2    size,
                                    Color      tint,
                                    TextureFilter filterMode,
                                    char externalAsset)
{
    Texture2D tex = GetTexture(path, filterMode, externalAsset);
    if (tex.id == 0) return;

    // (Vector2){0,0} means "use the sourceRect's pixel dimensions"
    float drawW = (size.x > 0.0f) ? size.x : sourceRect.width;
    float drawH = (size.y > 0.0f) ? size.y : sourceRect.height;

    Rectangle destRect = { position.x, position.y, drawW, drawH };
    Vector2   origin   = { 0.0f, 0.0f };

    DrawTexturePro(tex, sourceRect, destRect, origin, 0.0f, tint);
}

void TextureManager_Unload(const char *path)
{
    Cache_Init();

    // A single image can be cached under multiple filter modes — unload all of them.
    for (int i = 0; i < TEXTURE_CACHE_MAX; i++) {
        if (s_cache[i].occupied && strcmp(s_cache[i].path, path) == 0) {
            UnloadTexture(s_cache[i].texture);
            memset(&s_cache[i], 0, sizeof(TextureCacheEntry));
            TraceLog(LOG_INFO, "TEXTUREMGR: Unloaded '%s'", path);
        }
    }
}

void TextureManager_UnloadAll(void)
{
    Cache_Init();

    for (int i = 0; i < TEXTURE_CACHE_MAX; i++) {
        if (s_cache[i].occupied) {
            UnloadTexture(s_cache[i].texture);
            memset(&s_cache[i], 0, sizeof(TextureCacheEntry));
        }
    }

    TraceLog(LOG_INFO, "TEXTUREMGR: Unloaded all cached textures.");
}