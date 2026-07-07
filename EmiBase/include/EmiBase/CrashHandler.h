#pragma once

typedef void (*CrashHandlerFn)(void* crashInfo);

void CrashHandler_Init();
#ifdef EMIBASE_INTERNAL
    void _crashhandler_internal_sendstring(const char* status);
    void _crashhandler_internal_sendstatus(size_t status);
#endif