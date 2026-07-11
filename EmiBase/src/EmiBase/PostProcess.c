#include "EmiBase.h"
#include "rlgl.h"
#include "EmiBase/NuklearUI.h"

#include <stdio.h>

#if SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0
    Shader globalCRTShader;

    static SceneShaderList registry[MAX_SCENES];
    static int registryCount = 0;
    static int frame = 0;

    static RenderTexture2D pingpong;

    static SceneShaderList *GetList(Scene *s)
    {
        for (int i = 0; i < registryCount; i++)
            if (registry[i].scene == s) return &registry[i];
        return NULL;
    }

    static void ApplyPass(ShaderSlot *s, RenderTexture2D *src, float time, int w, int h)
    {
        Vector2 res = { w, h };
        if(s->timeLoc != -1) SetShaderValue(*s->shader, s->timeLoc, &time, SHADER_UNIFORM_FLOAT);
        if(s->resLoc != -1) SetShaderValue(*s->shader, s->resLoc,  &res,  SHADER_UNIFORM_VEC2);
        if(s->frameLoc != -1) SetShaderValue(*s->shader, s->frameLoc,  &frame,  SHADER_UNIFORM_INT);

        BeginShaderMode(*s->shader);
            DrawTextureRec(
                src->texture,
                (Rectangle){ 0, 0, src->texture.width, -src->texture.height },
                (Vector2){ 0, 0 },
                WHITE
            );
        EndShaderMode();
    }

    int PostProcess_Init(int screenWidth, int screenHeight)
    {
        pingpong = LoadRenderTexture(screenWidth, screenHeight);

        if(!IsRenderTextureValid(pingpong))
        {
            eprintf("[PostProcess] Failed to init RenderTextures\n");
            return 0;
        }

        eprintf("[PostProcess] Ready!\n");
        return 1;
    }

    void PostProcess_Cleanup()
    {
        UnloadRenderTexture(pingpong);
    }

    void PostProcess_Resize(int screenWidth, int screenHeight)
    {
        UnloadRenderTexture(pingpong);
        pingpong = LoadRenderTexture(screenWidth, screenHeight);
        if(!IsRenderTextureValid(pingpong))
        {
            eprintf("[PostProcess] Runtime error resizing window\n");
            return;
        }
    }

    void PostProcess_RegisterScene(Scene *s)
    {
        if (registryCount < MAX_SCENES) {
            registry[registryCount++] = (SceneShaderList){ .scene = s, .count = 0 };
        } else {
            eprintf("[PostProcess] Scene list is full, cannot RegisterScene!\n");
        }
    }

    void PostProcess_AddShader(Scene *s, Shader *shader)
    {
        SceneShaderList *list = GetList(s);
        if (!list) { eprintf("[PostProcess] Attempted to add a shader to an unregistered scene!\n"); return; }
        int timeLoc = GetShaderLocation(*shader, "time");
        int resLoc = GetShaderLocation(*shader, "resolution");
        int frameLoc = GetShaderLocation(*shader, "frame");
        if (list->count < MAX_SCENE_SHADERS) {
            list->slots[list->count++] = (ShaderSlot){ .shader = shader, .active = true, .timeLoc = timeLoc, .resLoc = resLoc, .frameLoc = frameLoc };
        }
    }

    void PostProcess_SetActive(Scene *s, Shader *shader, bool active)
    {
        SceneShaderList *list = GetList(s);
        if (!list) return;
        for (int i = 0; i < list->count; i++) {
            if (list->slots[i].shader == shader) {
                list->slots[i].active = active;
                return;
            }
        }
    }

    void PostProcess_Apply(RenderTexture2D *sceneTarget, Scene *top, float time, int screenWidth, int screenHeight, void (*overlay)())
    {
        frame++;
        
        SceneShaderList *list = top ? GetList(top) : NULL;

        // No shaders registered — draw the scene as-is
        if (!list || list->count == 0) {
            BeginDrawing();
                rlClearScreenBuffers();
                DrawTextureRec(
                    sceneTarget->texture,
                    (Rectangle){ 0, 0, sceneTarget->texture.width, -sceneTarget->texture.height },
                    (Vector2){ 0, 0 },
                    WHITE
                );
            if (overlay) overlay();
            EndDrawing();
            return;
        }

        // Build pass list from scene's active shaders only
        ShaderSlot *passes[MAX_SCENE_SHADERS];
        int passCount = 0;

        for (int i = 0; i < list->count; i++)
            if (list->slots[i].active)
                passes[passCount++] = &list->slots[i];

        RenderTexture2D *src = sceneTarget;
        bool drawToMainTarget = false;

        if(nk_postProcess == 0)
        {
            for (int i = 0; i < passCount; i++) {
                bool isLast = (i == passCount - 1);

                if (isLast) {
                    if(passCount > 1) rlDisableFramebuffer();
                    BeginDrawing();
                    rlClearScreenBuffers();
                    ApplyPass(passes[i], src, time, screenWidth, screenHeight);
                } else {
                    RenderTexture2D *target = (drawToMainTarget ? sceneTarget : &pingpong);
                    rlEnableFramebuffer(target->id);
                        rlClearScreenBuffers();
                        ApplyPass(passes[i], src, time, screenWidth, screenHeight);
                    src = target;
                    drawToMainTarget = !drawToMainTarget;
                }
            }
        } else {
            BeginDrawing();
            rlClearScreenBuffers();
            DrawTextureRec(
                src->texture,
                (Rectangle){ 0, 0, src->texture.width, -src->texture.height },
                (Vector2){ 0, 0 },
                WHITE
            );
        }

        if (overlay) overlay();
        EndDrawing();
    }
#else
    int PostProcess_Init(int screenWidth, int screenHeight) { eprintf("[PostProcess] Module disabled in compilation\n"); }
    void PostProcess_Cleanup() {}
    void PostProcess_Resize(int screenWidth, int screenHeight) {}
    void PostProcess_RegisterScene(Scene *s) {}
    void PostProcess_AddShader(Scene *s, Shader *shader) {}
    void PostProcess_SetActive(Scene *s, Shader *shader, bool active) {}
    void PostProcess_Apply(RenderTexture2D *sceneTarget, Scene *top, float time, int screenWidth, int screenHeight, void (*overlay)()) {}
#endif