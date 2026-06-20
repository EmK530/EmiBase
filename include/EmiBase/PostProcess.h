#pragma once
#include "raylib.h"
#include "SceneUtils.h"
#include <stdbool.h>

#define MAX_SCENE_SHADERS 8

typedef struct {
    Shader  *shader;
    bool     active;
} ShaderSlot;

typedef struct {
    Scene    *scene;
    ShaderSlot slots[MAX_SCENE_SHADERS];
    int      count;
} SceneShaderList;

void PostProcess_Init(int screenWidth, int screenHeight);
void PostProcess_Cleanup();

void PostProcess_RegisterScene(Scene *s);
void PostProcess_AddShader(Scene *s, Shader *shader);
void PostProcess_SetActive(Scene *s, Shader *shader, bool active);
void PostProcess_Resize(int screenWidth, int screenHeight);

void PostProcess_Apply(RenderTexture2D *sceneTarget, Scene *top, float time, int screenWidth, int screenHeight, void (*overlay)());