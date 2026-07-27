#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define EMIBASE_INTERNAL

#include "EmiBase.h"
#include "rlgl.h"
#if RAYLIB_VERSION_MAJOR < 6
    #error "EmiBase was developed for raylib 6.0 and above, please upgrade your version of raylib."
#endif
#include "EmiBase/CrashHandler.h"
#include "EmiBase/NuklearUI.h" // Don't worry, it's not part of Release.

static bool detached = false;
static bool initialized = false;

static int fixedScreenWidth = -1;
static int fixedScreenHeight = -1;
static int queuedResizeX = -1;
static int queuedResizeY = -1;
static bool isCustomRenderTarget = false;
static RenderTexture2D target;

extern int Game_PreInit();
extern int Game_Initialize();

int EmiBase_Init()
{
    Game_PreInit();
    CrashHandler_Init();
    _crashhandler_internal_sendstatus(2);
    if(!ContentManager_Init(CONTENT_NAME)) return 2;
    SetConfigFlags(RAYLIB_FLAGS);
    InitWindow(RES_X, RES_Y, WINDOW_TITLE);
    SetDarkTitleBar();
    if(!IsWindowReady())
    {
        eprintf("[EmiBase] Failed to create a game window.\n");
        WinMessageBox("Fatal error!", "Failed to create a game window.", MB_TOPMOST | MB_ICONERROR);
        return 2;
    }

    int targetWidth = fixedScreenWidth == -1 ? RES_X : fixedScreenWidth;
    int targetHeight = fixedScreenHeight == -1 ? RES_Y : fixedScreenHeight;

    if(!EmiObject_Init()) return 1;
    if(!PostProcess_Init(targetWidth, targetHeight)) return 1;
    if(!AudioManager_Init()) return 1;

    if(fixedScreenWidth != -1 || (SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0))
    {
        target = LoadRenderTexture(targetWidth, targetHeight);
        isCustomRenderTarget = true;
        if(!IsRenderTextureValid(target))
        {
            eprintf("[EmiBase] Could not create a RenderTexture target.\n");
            return 1;
        }
    }

    SetExitKey(KEY_NULL);
    SetTargetFPS(FPS_LIMIT);

    Game_Initialize(); // Call the Game entrypoint to load scenes.

    Scene *start = find_scene(STARTUP_SCENE);
    if (start) {
        PushScene(start);
    } else {
        eprintf("[EmiBase] Could not find startup scene: '" STARTUP_SCENE "'\n");
        WinMessageBox("Fatal error!", "The game's default scene could not be found!\nFailed to load '" STARTUP_SCENE "'", MB_TOPMOST | MB_ICONERROR);
        return 2;
    };

    _crashhandler_internal_sendstatus(3);

#ifndef RELEASE
    if(!NuklearUI_Init())
    {
        eprintf("[EmiBase] Failed to initialize Nuklear UI.\n");
        WinMessageBox("Fatal error!", "Failed to initialize Nuklear UI.", MB_TOPMOST | MB_ICONERROR);
    }
#endif

    _crashhandler_internal_sendstatus(0);

    initialized = true;

    return 0;
}

int EmiBase_GetScreenWidth() { return fixedScreenWidth == -1 ? GetScreenWidth() : fixedScreenWidth; }
int EmiBase_GetScreenHeight() { return fixedScreenHeight == -1 ? GetScreenHeight() : fixedScreenHeight; }

void EmiBase_SetRenderResolution(int width, int height)
{
    if(width < 1 || height < 1)
        return;
    if(!initialized)
    {
        fixedScreenWidth = width;
        fixedScreenHeight = height;
        return;
    }
    queuedResizeX = width;
    queuedResizeY = height;
}

void EmiBase_UnsetRenderResolution()
{
    queuedResizeX = 0;
    queuedResizeY = 0;
}

void EmiBase_ProcessInput()
{
    int key;
    while ((key = GetKeyPressed()) != 0)
    {
        for (int i = 0; i <= scene_stack.top; i++) {
            Scene *s = scene_stack.scenes[i];
            if (s && s->OnInput) {
                _crashhandler_internal_sendstring(s->name);
                s->OnInput(s, key);
                _crashhandler_internal_sendstatus(0);
            }
        }
    }
}

#if SOFTWARE_OPTIMIZATIONS == 1
    extern void *swGetColorBuffer(int *width, int *height);
    void ClearBackgroundRLSW()
    {
        int width, height;
        uint32_t *pixels = (uint32_t *)swGetColorBuffer(&width, &height);
        memset(pixels, 0, (size_t)width * height * sizeof(uint32_t));
    }
    static bool shouldClear = true;
    void EmiBase_SetScreenClearEnabled(bool enabled)
    {
        shouldClear = enabled;
    }
#endif

void EmiBase_BeginDrawing()
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    if(isCustomRenderTarget)
    {
        if (fixedScreenWidth == -1 && IsWindowResized())
        {
            PostProcess_Resize(screenWidth, screenHeight);
            UnloadRenderTexture(target);
            target = LoadRenderTexture(screenWidth, screenHeight);
            BeginTextureMode(target);
        } else {
            BeginTextureMode(target);
            rlClearScreenBuffers();
        }
    } else {
        BeginDrawing();
    #if SOFTWARE_OPTIMIZATIONS == 1
    if(shouldClear)
        ClearBackgroundRLSW();
    #else
        DrawRectangle(0, 0, screenWidth, screenHeight, BLACK); // This performs better than ClearBackground in software???
    #endif
    }
}

static void _emibase_restore_framebuffer(unsigned int fbo, int width, int height)
{
    rlDrawRenderBatchActive();
    rlEnableFramebuffer(fbo);
    rlViewport(0, 0, width, height);
    rlSetFramebufferWidth(width);
    rlSetFramebufferHeight(height);
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    rlOrtho(0, width, height, 0, 0.0f, 1.0f);
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

void EmiBase_Detach()
{
    if(detached)
    {
        eprintf("[EmiBase] Attempt to detach renderer when already detached!");
        return;
    }
    detached = true;
    rlDrawRenderBatchActive();
}

void EmiBase_DetachedTextureMode(RenderTexture2D tex)
{
    if(!detached)
    {
        eprintf("[EmiBase] Attempt to enable detached texture mode when not detached!");
        return;
    }
    _emibase_restore_framebuffer(tex.id, tex.texture.width, tex.texture.height);
}

RenderTexture2D EmiBase_LoadRenderTexture(int width, int height)
{
    RenderTexture2D tex = { 0 };

    tex.id = rlLoadFramebuffer();

    if (tex.id > 0)
    {
        rlEnableFramebuffer(tex.id);

        tex.texture.id = rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        tex.texture.width = width;
        tex.texture.height = height;
        tex.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        tex.texture.mipmaps = 1;

        tex.depth.id = rlLoadTextureDepth(width, height, true);
        tex.depth.width = width;
        tex.depth.height = height;
        tex.depth.format = 19;
        tex.depth.mipmaps = 1;

        rlFramebufferAttach(tex.id, tex.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(tex.id, tex.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        if (rlFramebufferComplete(tex.id))
            TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", tex.id);

#if SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0
        rlEnableFramebuffer(target.id);
#else
        rlEnableFramebuffer(0);
#endif
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return tex;
}

void EmiBase_Attach()
{
    if(!detached)
    {
        eprintf("[EmiBase] Attempt to attach renderer when already attached!");
        return;
    }
    detached = false;
#if SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0
    _emibase_restore_framebuffer(target.id, target.texture.width, target.texture.height);
#else
    _emibase_restore_framebuffer(0, GetScreenWidth(), GetScreenHeight());
#endif
}

#ifndef RELEASE
    void (*overlayRemember)() = NULL;
    void doubleDraw() {
        if(nk_overlay == 0 && overlayRemember != NULL)
            overlayRemember();
        _crashhandler_internal_sendstatus(4);
        NuklearUI_Draw();
        _crashhandler_internal_sendstatus(0);
    }
    void EmiBase_EndDrawing(void (*overlay)())
    {
        AudioManager_Update();
        if(isCustomRenderTarget) {
            int screenWidth = EmiBase_GetScreenWidth();
            int screenHeight = EmiBase_GetScreenHeight();
            rlDrawRenderBatchActive();
            EndTextureMode();
            #if SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0
                overlayRemember = overlay;
                PostProcess_Apply(&target, TopScene(), GetTime(), screenWidth, screenHeight, doubleDraw);
                goto shouldResize;
            #else
                BeginDrawing();
                rlClearScreenBuffers();
                float screenW = GetScreenWidth(); float screenH = GetScreenHeight();
                float w = target.texture.width; float h = target.texture.height;
                float scale = fminf(screenW / (float)w, screenH / (float)h);
                float drawW = w * scale; float drawH = h * scale;
                DrawTexturePro(target.texture,
                    (Rectangle){ 0, 0, target.texture.width, -target.texture.height },
                    (Rectangle){ (screenW - drawW) * 0.5f, (screenH - drawH) * 0.5f, drawW, drawH },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
            #endif
        }
        if(nk_overlay == 0 && overlay != NULL)
            overlay();
        _crashhandler_internal_sendstatus(4);
        NuklearUI_Draw();
        _crashhandler_internal_sendstatus(0);
        EndDrawing();

        shouldResize:
        if(queuedResizeX != 1 && queuedResizeY != -1)
        {
            if(queuedResizeX == 0)
            {
                if(fixedScreenWidth == -1)
                    return;
                if(isCustomRenderTarget) {
                    UnloadRenderTexture(target);
                    if(SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0)
                    {
                        PostProcess_Resize(GetScreenWidth(), GetScreenHeight());
                        target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
                        fixedScreenWidth = -1;
                        fixedScreenHeight = -1;
                    } else {
                        isCustomRenderTarget = false;
                    }
                }
            } else if(queuedResizeX != fixedScreenWidth && queuedResizeY != fixedScreenHeight) {
                if(isCustomRenderTarget)
                    UnloadRenderTexture(target);
                target = LoadRenderTexture(queuedResizeX, queuedResizeY);
                if(SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0)
                    PostProcess_Resize(queuedResizeX, queuedResizeY);
                isCustomRenderTarget = true;
                fixedScreenWidth = queuedResizeX;
                fixedScreenHeight = queuedResizeY;
            }
            queuedResizeX = -1;
            queuedResizeY = -1;
        }
    }
#else
    void EmiBase_EndDrawing(void (*overlay)())
    {
        AudioManager_Update();
    #if SUPPORTS_POSTPROCESS == 1 && SOFTWARE_OPTIMIZATIONS == 0
            int screenWidth = EmiBase_GetScreenWidth();
            int screenHeight = EmiBase_GetScreenHeight();
            rlDrawRenderBatchActive();
            rlDisableFramebuffer();
            PostProcess_Apply(&target, TopScene(), GetTime(), screenWidth, screenHeight, overlay);
    #else
            overlay();
            EndDrawing();
    #endif
    }
#endif

void _emibase_internal_replacescene(Scene* target)
{
    SceneStack new_stack = { .top = -1 };
    scene_stack = new_stack;
    for(int i = 0; i < MAX_SCENES; i++)
    {
        Scene* scene = registered_scenes[i];
        if(scene == NULL)
            break;
        if(scene->active)
            scene->Cleanup(scene);
        scene->active = false;
    }
    PushScene(target);
}

void EmiBase_StepScenes()
{
    float deltaTime = GetFrameTime();
    int screenWidth = EmiBase_GetScreenWidth();
    int screenHeight = EmiBase_GetScreenHeight();

    if(nk_workEarly == 0)
    {
        for (int i = 0; i <= scene_stack.top; i++) {
            Scene *s = scene_stack.scenes[i];
            if (s && s->WorkEarly) {
                _crashhandler_internal_sendstring(s->name);
                SceneResult res = s->WorkEarly(s, deltaTime);
                if(detached)
                {
                    eprintf("[EmiBase] Scene '%s' ended without re-attaching the renderer in WorkEarly!\n", s->name);
                    EmiBase_Attach();
                }
                _crashhandler_internal_sendstatus(0);

                if (res.action != SCENE_NONE && res.name) {
                    Scene *new_scene = find_scene(res.name);

                    if (new_scene) {
                        if (res.action == SCENE_PUSH) {
                            PushScene(new_scene);
                        }
                        else if (res.action == SCENE_REPLACE) {
                            _emibase_internal_replacescene(new_scene);
                        }
                        else if (res.action == SCENE_POP) {
                            PopScene();
                        }
                    } else {
                        eprintf("[EmiBase] Failed to jump to nonexistent scene: %s\n", res.name);
                    }
                }
            }
        }
    }

    _crashhandler_internal_sendstatus(1);

    if(EmiObject_AutoDraw)
        EmiObject_Draw(screenWidth, screenHeight);

    _crashhandler_internal_sendstatus(0);

    if(nk_workLate == 0)
    {
        for (int i = 0; i <= scene_stack.top; i++) {
            Scene *s = scene_stack.scenes[i];
            if (s && s->WorkLate) {
                _crashhandler_internal_sendstring(s->name);
                SceneResult res = s->WorkLate(s, deltaTime);
                if(detached)
                {
                    eprintf("[EmiBase] Scene '%s' ended without re-attaching the renderer in WorkLate!\n", s->name);
                    EmiBase_Attach();
                }
                _crashhandler_internal_sendstatus(0);
                if (res.action != SCENE_NONE && res.name) {
                    Scene *new_scene = find_scene(res.name);

                    if (new_scene) {
                        if (res.action == SCENE_PUSH) {
                            PushScene(new_scene);
                        } else if (res.action == SCENE_REPLACE) {
                            _emibase_internal_replacescene(new_scene);
                        } else if (res.action == SCENE_POP) {
                            PopScene();
                        }
                    } else {
                        eprintf("[EmiBase] Failed to jump to nonexistent scene: %s\n", res.name);
                    }
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

static char message[128];
void RaiseAllocationError(size_t bytes)
{
    snprintf(message, 128, "Ran out of memory or heap is fragmented.\n\nEmiBase failed to allocate %i bytes.", bytes);
    WinMessageBox("Fatal error!", message, MB_TOPMOST | MB_ICONERROR);
    WinExitProcess(1);
}

void* emalloc_strict(size_t bytes)
{
    void* alloc = RL_MALLOC(bytes);
    if(!alloc)
    {
        RaiseAllocationError(bytes);
        return NULL;
    }
    return alloc;
}

void* ecalloc_strict(size_t num, size_t bytes)
{
    void* alloc = RL_CALLOC(num, bytes);
    if(!alloc)
    {
        RaiseAllocationError(bytes);
        return NULL;
    }
    return alloc;
}