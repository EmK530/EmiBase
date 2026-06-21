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
    #define PROJECT_NAME "EmiBase Engine"
#endif

// What scene to render on program start, needs to exist or EmiBase will fail to load.
#ifndef STARTUP_SCENE
    #define STARTUP_SCENE "Menu"
#endif

/*
    Compiles EmiBase with a Post Processing wrapper. Breaks software rendering.
    If disabled, all PostProcess functions will be replaced with dummy variants.
*/
#ifndef SUPPORTS_POSTPROCESS
    #define SUPPORTS_POSTPROCESS 1
#endif

// Build identifier to display as a suffix to the project name.
#ifndef BUILD_IDENT
    #if SUPPORTS_POSTPROCESS == 1
        #define BUILD_IDENT "[dev]"
    #else
        #define BUILD_IDENT "[dev:P0]"
    #endif
#endif

// I mean, this one is pretty straightforward.
#ifndef FPS_LIMIT
    #define FPS_LIMIT 1000
#endif

// Default window width in pixels.
#ifndef RES_X
    #define RES_X 1280
#endif

// Default window height in pixels.
#ifndef RES_Y
    #define RES_Y 720
#endif

// Removes all logging made by function calls to eprintf, strings should end up discarded by the compiler.
#ifndef NO_LOGGING
    #define NO_LOGGING 0
#endif

/*
    Determines the filename for ContentManager to load.
    You should probably update pak_builder.py if you change this.
*/
#define CONTENT_NAME "content.epak"

/* DO NOT EDIT BEYOND THIS POINT */

int EmiBase_Init();
void EmiBase_BeginDrawing();
void EmiBase_EndDrawing(void (*overlay)()); // Run at the end of your frame, overlay can be set to draw topmost graphics unaffected by shading.
void EmiBase_StepScenes(); // Execute scene code and draw EmiObjects
void EmiBase_ProcessInput(); // Sends queued input signals to the active scenes.
void EmiBase_Cleanup();

#if NO_LOGGING == 0
    void eprintf(const char* fmt, ...);
#else
    #define eprintf(...) ((void)0)
#endif