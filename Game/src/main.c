#include "EmiBase.h"

// Define all your scenes.
extern void Register_Demo();
extern void Register_Blank();

// This is ran before EmiBase initialization, here you can prepare a fixed resolution you want EmiBase to render at.
int Game_PreInit()
{

}

// This is ran during EmiBase initialization, common practice is to submit defined scenes here.
int Game_Initialize()
{
    // Loading all scenes.
    Register_Demo();
    Register_Blank();
}

void PostDraw_Overlay()
{
    DrawFPS(10, GetScreenHeight() - 27);
}