#pragma once

#include <stdint.h>
#include "EmiBase/CrashHandler.h"

#ifdef _WIN32
    typedef void* HWND;
    typedef long LONG;
    typedef int BOOL;
    typedef unsigned int UINT;
    typedef void* HANDLE;
    typedef unsigned long DWORD;
    typedef unsigned long long ULONG_PTR;
    typedef void* HANDLE;

    typedef struct _CONTEXT CONTEXT;

    typedef struct _EXCEPTION_RECORD {
        DWORD ExceptionCode;
        DWORD ExceptionFlags;
        struct _EXCEPTION_RECORD* ExceptionRecord;
        void* ExceptionAddress;
        DWORD NumberParameters;
        ULONG_PTR ExceptionInformation[15]; // Windows defines up to 15
    } EXCEPTION_RECORD;
    
    typedef struct _EXCEPTION_POINTERS {
        EXCEPTION_RECORD* ExceptionRecord;
        CONTEXT* ContextRecord;
    } EXCEPTION_POINTERS;

    HWND __stdcall GetActiveWindow(void);
    int __stdcall MessageBoxA(HWND hWnd, const char* lpText, const char* lpCaption, UINT uType);
    LONG __stdcall DwmSetWindowAttribute(HWND hwnd, UINT dwAttribute, const void* pvAttribute, UINT cbAttribute);
    LONG __stdcall SetUnhandledExceptionFilter(LONG (__stdcall *lpTopLevelExceptionFilter)(EXCEPTION_POINTERS*));

    void SetDarkTitleBar(void);
    void WinMessageBox(char* title, char* message, uint32_t type);
    void WinSetUnhandledExceptionFilter(CrashHandlerFn handler);
#else
    static inline void SetDarkTitleBar(void) {}
    #define WinMessageBox(...) ((void)0)
    static inline void WinSetUnhandledExceptionFilter(CrashHandlerFn handler) {}
#endif

#ifndef NULL
    #define NULL (void*)0
#endif

#ifndef EXCEPTION_ACCESS_VIOLATION
    #define EXCEPTION_ACCESS_VIOLATION      0xC0000005L
    #define EXCEPTION_STACK_OVERFLOW        0xC00000FDL
    #define EXCEPTION_ILLEGAL_INSTRUCTION   0xC000001DL
    #define EXCEPTION_INT_DIVIDE_BY_ZERO    0xC0000094L
    #define EXCEPTION_FLT_DIVIDE_BY_ZERO    0xC000008EL
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