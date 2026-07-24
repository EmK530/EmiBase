#pragma once

#include "EmiObject/EmiObject.h"

#include "EmiBase/AudioManager.h"
#include "EmiBase/ContentManager.h"
#include "EmiBase/FontManager.h"
#include "EmiBase/PostProcess.h"
#include "EmiBase/SceneUtils.h"
#include "EmiBase/TextureManager.h"

#include "EmiBase/easings.h"

#include "Libraries/WinAPI.h"

#include "raylib.h"

////////////////////////////////////////
/*          PROJECT SETTINGS          */
////////////////////////////////////////

#ifndef PROJECT_NAME// Internal project name, referenced by the engine.
    #define PROJECT_NAME "Example Project"
#endif

#ifndef PROJECT_VER// Internal project version, referenced by the engine.
    #define PROJECT_VER "v0.0.0"
#endif

#ifndef WINDOW_TITLE// Title of the game window that gets created on boot.
    #define WINDOW_TITLE "Example Project"
#endif

#ifndef RAYLIB_FLAGS// Raylib config flags that will be applied before EmiBase initializes the game window.
    #define RAYLIB_FLAGS FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE
#endif

#ifndef STARTUP_SCENE// What scene to load when the program starts, EmiBase closes if no scene is found.
    #define STARTUP_SCENE "Demo"
#endif

#ifndef FPS_LIMIT// FPS limit to be applied by Raylib if not already limited by VSync.
    #define FPS_LIMIT 1000
#endif

#ifndef RES_X// Startup window width in pixels
    #define RES_X 1600
#endif

#ifndef RES_Y// Startup window height in pixels
    #define RES_Y 900
#endif

////////////////////////////////////////
/*          ENGINE SETTINGS           */
////////////////////////////////////////

#ifndef SOFTWARE_OPTIMIZATIONS// Tweaks EmiBase for faster software (CPU) rendering. Disables Post Processing.
    #define SOFTWARE_OPTIMIZATIONS 0
#endif

#ifndef DEFAULT_FONT// Default font that EText objects are created with
    #define DEFAULT_FONT "EmiBase/ArialCE.ttf"
#endif

#ifndef DEFAULT_IMAGE// Default image that EImage objects are created with
    #define DEFAULT_IMAGE "EmiBase/EImage.png"
#endif

#ifndef SUPPORTS_POSTPROCESS// Enables the EmiBase PostProcess module, slight overhead but allows layered shaders on the final render output.
    #define SUPPORTS_POSTPROCESS 1
#endif

#ifndef CONTENT_NAME// The filename ContentManager tries to open to load game assets.
    #define CONTENT_NAME "content.epak"
#endif

#ifndef CONTENT_KEY// Controls the key that ContentManager tries to read encrypted game packages with.
    #define CONTENT_KEY 0xC417A251
#endif

////////////////////////////////////////
/*           MISC SETTINGS            */
////////////////////////////////////////

// Removes all logging made by function calls to eprintf, strings should end up discarded by the compiler.
#ifndef NO_LOGGING
    #ifdef RELEASE
        #define NO_LOGGING 1
    #else
        #define NO_LOGGING 0
    #endif
#endif

/* DO NOT EDIT BEYOND THIS POINT */

#ifdef CONTEXT_EMIMAIN
    int EmiBase_Init();
    void EmiBase_BeginDrawing();
    void EmiBase_EndDrawing(void (*overlay)()); // Run at the end of your frame, overlay can be set to draw topmost graphics unaffected by shading.
    void EmiBase_StepScenes(); // Execute scene code and draw EmiObjects
    void EmiBase_ProcessInput(); // Sends queued input signals to the active scenes.
    void EmiBase_Cleanup();
#endif
#if SOFTWARE_OPTIMIZATIONS == 1
    void EmiBase_SetScreenClearEnabled(bool enabled); // Toggles screen clearing, necessary to disable for performance if your elements draw over the entire screen.
#endif
void EmiBase_Detach(); // EmiBase-safe version of EndDrawing to transfer full rendering control to a scene. Remember to call Attach after you are done.
void EmiBase_DetachedTextureMode(RenderTexture2D tex); // EmiBase-safe version of BeginTextureMode to draw in a RenderTexture without flushing the screen.
RenderTexture2D EmiBase_LoadRenderTexture(int width, int height); // EmiBase-safe version of LoadRenderTexture that ensures Detach does not break.
void EmiBase_Attach(); // Return rendering control back to EmiBase, needs to be called before the end of a scene if you ran Detach.

#define EMIBASE_VER "v1.1.0"

#ifndef GIT_HASH
    #define GIT_HASH "00000000"
#endif
#ifndef GIT_DIRTY
    #define GIT_DIRTY ""
#endif
#ifndef GIT_BRANCH
    #define GIT_BRANCH ""
#endif

#if NO_LOGGING == 0
    void eprintf(const char* fmt, ...);
#else
    #define eprintf(...) ((void)0)
#endif