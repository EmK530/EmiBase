#pragma once

// Draw a texture from a path. Loads and caches on first use.
// path       - path to image file, relative to working directory
// position   - top-left position in screen space
// size       - desired draw size in pixels; pass (Vector2){0,0} to use the texture's native size
// tint       - colour tint + alpha (use WHITE for no tint, set .a for transparency)
// filterMode - TEXTURE_FILTER_POINT or TEXTURE_FILTER_BILINEAR
void TextureManager_DrawTexture(const char *path,
                                Vector2    position,
                                Vector2    size,
                                Color      tint,
                                TextureFilter filterMode,
                                char externalAsset);

// Draw a sub-region of a texture (spritesheet / atlas support).
// sourceRect - rectangle within the source image to draw
// size       - desired draw size in pixels; pass (Vector2){0,0} to use the sourceRect's size
void TextureManager_DrawTextureRect(const char *path,
                                    Rectangle  sourceRect,
                                    Vector2    position,
                                    Vector2    size,
                                    Color      tint,
                                    TextureFilter filterMode,
                                    char externalAsset);

// Explicitly release a single cached texture.
// Safe to call with a path that was never loaded (no-op).
void TextureManager_Unload(const char *path);

// Release every texture currently held in the cache.
// Call before CloseWindow(), or when changing scenes.
void TextureManager_UnloadAll(void);