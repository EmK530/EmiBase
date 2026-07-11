#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define CONTEXT_EMIMAIN
#include "EmiBase.h"
#include "rlgl.h"

// Road ends here :)

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

extern void PostDraw_Overlay();

int main()
{
    int initResult = EmiBase_Init();
    if(initResult != 0) {
        eprintf("[Main] Could not initialize EmiBase\n");
        if(initResult == 1) WinMessageBox("Fatal error!", "Could not initialize EmiBase systems!", MB_TOPMOST | MB_ICONERROR);
        WinExitProcess(1);
        return 1;
    }

    eprintf("[EmiBase] Initialized!\n");

    int frames = 0;
    while (!WindowShouldClose())
    {
        EmiBase_ProcessInput();
        EmiBase_BeginDrawing();

        EmiBase_StepScenes();

        EmiBase_EndDrawing(PostDraw_Overlay);
    }

    EmiBase_Cleanup();
    CloseWindow();
}