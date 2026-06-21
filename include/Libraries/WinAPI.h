#pragma once

#include "EmiBase/CrashHandler.h"

#ifdef _WIN32
#include <stdint.h>
void SetDarkTitleBar(void);
void WinMessageBox(char* title, char* message, uint32_t type);
void WinSetUnhandledExceptionFilter(CrashHandlerFn handler);
#else
static inline void SetDarkTitleBar(void) {}
#define WinMessageBox(...) ((void)0)
static inline void WinSetUnhandledExceptionFilter(CrashHandlerFn handler) {}
#endif

#ifndef MB_ABORTRETRYIGNORE
    #define MB_ABORTRETRYIGNORE 0x00000002L
    #define MB_CANCELTRYCONTINUE 0x00000006L
    #define MB_HELP 0x00004000L
    #define MB_OK 0x00000000L
    #define MB_OKCANCEL 0x00000001L
    #define MB_RETRYCANCEL 0x00000005L
    #define MB_YESNO 0x00000004L
    #define MB_YESNOCANCEL 0x00000003L

    #define MB_ICONWARNING 0x00000030L
    #define MB_ICONINFORMATION 0x00000040L
    #define MB_ICONQUESTION 0x00000020L
    #define MB_ICONERROR 0x00000010L

    #define MB_TOPMOST 0x00040000L
#endif