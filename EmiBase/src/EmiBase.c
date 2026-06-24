#include <stdio.h>
#include <stdarg.h>

#include "EmiBase.h"

#if SUPPORTS_POSTPROCESS == 1
RenderTexture2D target;
#endif

extern int main();

int EmiBase_Init()
{
    CrashHandler_Init();
    _crashhandler_internal_sendstatus(2);
    if(!ContentManager_Init(CONTENT_NAME)) return 2;
    InitWindow(RES_X, RES_Y, PROJECT_NAME " " BUILD_IDENT);
    SetDarkTitleBar();
    if(!IsWindowReady())
    {
        eprintf("[EmiBase] Failed to create a game window.\n");
        WinMessageBox("Fatal error!", "Failed to create a game window.", MB_TOPMOST | MB_ICONERROR);
        return 2;
    }
    if(!EmiObject_Init()) return 1;
    if(!PostProcess_Init(RES_X, RES_Y)) return 1;
    if(!AudioManager_Init()) return 1;

#if SUPPORTS_POSTPROCESS == 1
    target = LoadRenderTexture(RES_X, RES_Y);
    if(!IsRenderTextureValid(target))
    {
        eprintf("[EmiBase] Could not create a RenderTexture target for PostProcess.\n");
        return 1;
    }
#endif

    SetTargetFPS(FPS_LIMIT);

    main(); // Call the Game entrypoint to load scenes.

    Scene *start = find_scene(STARTUP_SCENE);
    if (start) {
        PushScene(start);
    } else {
        eprintf("[EmiBase] Could not find startup scene: '" STARTUP_SCENE "'\n");
        return 1;
    };

    _crashhandler_internal_sendstatus(0);

    return 0;
}

void EmiBase_ProcessInput()
{
    Scene *top = TopScene();
    if (top && top->OnInput) {
        int key;
        while ((key = GetKeyPressed()) != 0)
            top->OnInput(top, key);
    }
}

void EmiBase_BeginDrawing()
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
#if SUPPORTS_POSTPROCESS == 1
    if (IsWindowResized())
    {
        PostProcess_Resize(screenWidth, screenHeight);
        UnloadRenderTexture(target);
        target = LoadRenderTexture(screenWidth, screenHeight);
    }
    BeginTextureMode(target);
#else
    BeginDrawing();
#endif
}

void EmiBase_EndDrawing(void (*overlay)())
{
    AudioManager_Update();
#if SUPPORTS_POSTPROCESS == 1
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        EndTextureMode();
        PostProcess_Apply(&target, TopScene(), GetTime(), screenWidth, screenHeight, overlay);
#else
        overlay();
        EndDrawing();
#endif
}

void EmiBase_StepScenes()
{
    float deltaTime = GetFrameTime();
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    for (int i = 0; i <= scene_stack.top; i++) {
        Scene *s = scene_stack.scenes[i];
        if (s && s->WorkEarly) {
            _crashhandler_internal_sendstring(s->name);
            SceneResult res = s->WorkEarly(s, deltaTime);
            _crashhandler_internal_sendstatus(0);

            if (res.action != SCENE_NONE && res.name) {
                Scene *new_scene = find_scene(res.name);

                if (new_scene) {
                    if (res.action == SCENE_PUSH) {
                        PushScene(new_scene);
                    }
                    else if (res.action == SCENE_REPLACE) {
                        PopScene(); // remove current
                        PushScene(new_scene);
                    }
                } else {
                    eprintf("[EmiBase] Failed to jump to nonexistent scene: %s\n", res.name);
                }
            }
        }
    }

    _crashhandler_internal_sendstatus(1);

    EmiObject_Draw(screenWidth, screenHeight);

    _crashhandler_internal_sendstatus(0);

    for (int i = 0; i <= scene_stack.top; i++) {
        Scene *s = scene_stack.scenes[i];
        if (s && s->WorkLate) {
            _crashhandler_internal_sendstring(s->name);
            SceneResult res = s->WorkLate(s, deltaTime);
            _crashhandler_internal_sendstatus(0);
            if (res.action != SCENE_NONE && res.name) {
                Scene *new_scene = find_scene(res.name);

                if (new_scene) {
                    if (res.action == SCENE_PUSH) {
                        PushScene(new_scene);
                    }
                    else if (res.action == SCENE_REPLACE) {
                        PopScene();          // remove current
                        PushScene(new_scene);
                    }
                } else {
                    eprintf("[EmiBase] Failed to jump to nonexistent scene: %s\n", res.name);
                }
            }
        }
    }
}

void EmiBase_Cleanup()
{
    PostProcess_Cleanup();
}

#if NO_LOGGING == 0
    void eprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
#endif