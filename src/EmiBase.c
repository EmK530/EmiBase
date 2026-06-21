#include <stdio.h>
#include <stdarg.h>

#include "EmiBase.h"

int EmiBase_Init(int screenWidth, int screenHeight)
{
    if(ContentManager_Init(CONTENT_NAME)==0) return 2;
    if(!EmiObject_Init()) return 1;
    if(!PostProcess_Init(screenWidth, screenHeight)) return 1;
    if(!AudioManager_Init()) return 1;
    return 0;
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