#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "raylib.h"
#include "rlgl.h"
#include "EmiBase.h"

static void DrawOverlay()
{
    DrawFPS(10, GetScreenHeight() - 27);
    // Add anything else you may want
}

// Road ends here :)

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

extern void Register_MenuScene();

int main()
{
    Register_MenuScene();

    //SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int initResult = EmiBase_Init();
    if(initResult != 0) {
        eprintf("[Main] Could not initialize EmiBase\n");
        if(initResult == 1) WinMessageBox("Fatal error!", "Could not initialize EmiBase systems!", MB_TOPMOST | MB_ICONERROR);
        return 1;
    }

    eprintf("[Main] Ready!\n");

    while (!WindowShouldClose())
    {
        EmiBase_ProcessInput();
        EmiBase_BeginDrawing();

        ClearBackground(BLACK);
        EmiBase_StepScenes();

        EmiBase_EndDrawing(DrawOverlay);
    }

    EmiBase_Cleanup();
    CloseWindow();
    return 0;
}