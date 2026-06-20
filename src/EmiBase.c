#include <stdio.h>
#include "EmiBase.h"

int EmiBase_Init(int screenWidth, int screenHeight)
{
    if(ContentManager_Init(CONTENT_NAME)==0) return 2;
    if(!EmiObject_Init()) return 1;
    PostProcess_Init(screenWidth, screenHeight);
    AudioManager_Init();
    return 0;
}