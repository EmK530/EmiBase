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

// To configure the pak file in ContentManager, view tools/pak_builder.py

// Project name to display in the game window.
#ifndef PROJECT_NAME
    #define PROJECT_NAME "Example Project"
#endif

#ifndef PROJECT_VER
    #define PROJECT_VER "v0.0.0"
#endif

#ifndef WINDOW_TITLE
    #define WINDOW_TITLE "Example Project"
#endif

// Raylib config flags that will be applied before EmiBase initializes the game window.
#ifndef RAYLIB_FLAGS
    #define RAYLIB_FLAGS FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE
#endif

// What scene to render on program start, needs to exist or EmiBase will fail to load.
#ifndef STARTUP_SCENE
    #define STARTUP_SCENE "Demo"
#endif

// Default font that EText objects are created with
#ifndef DEFAULT_FONT
    #define DEFAULT_FONT "EmiBase/ArialCE.ttf"
#endif

// Default image that EImage objects are created with
#ifndef DEFAULT_IMAGE
    #define DEFAULT_IMAGE "EmiBase/EImage.png"
#endif

// Enables various EmiBase tweaks to significantly speed up the performance of software rendering. Disables Post Processing.
#ifndef SOFTWARE_OPTIMIZATIONS
    #define SOFTWARE_OPTIMIZATIONS 0
#endif

/*
    Compiles EmiBase with a Post Processing wrapper. Breaks software rendering and can lower framerate.
    If disabled, all PostProcess functions will be replaced with dummy variants.
*/
#ifndef SUPPORTS_POSTPROCESS
    #define SUPPORTS_POSTPROCESS 1
#endif

// I mean, this one is pretty straightforward.
#ifndef FPS_LIMIT
    #define FPS_LIMIT 1000
#endif

// Default window width in pixels.
#ifndef RES_X
    #define RES_X 1600
#endif

// Default window height in pixels.
#ifndef RES_Y
    #define RES_Y 900
#endif

// Removes all logging made by function calls to eprintf, strings should end up discarded by the compiler.
#ifndef NO_LOGGING
    #ifdef RELEASE
        #define NO_LOGGING 1
    #else
        #define NO_LOGGING 0
    #endif
#endif

/*
    Determines the filename for ContentManager to load.
    You should probably update pak_builder.py if you change this.
*/
#define CONTENT_NAME "content.epak"

// The key used for reading encrypted content files, must match the key in the Python script but can be ignored if encryption is off.
#define CONTENT_KEY 0xC417A251

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