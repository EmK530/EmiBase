#pragma once

#include "EmiBase.h"

void FontManager_Init(void);
void FontManager_Shutdown(void);

Font* FontManager_GetFont(const char* path, int size);

void FontManager_DrawText(const char* path, int size,
                          const char* text,
                          Vector2 position,
                          Color color);