#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <stdint.h>

#include "Libraries/WinAPI.h"

#pragma comment(lib, "dwmapi.lib")

void SetDarkTitleBar(void)
{
    HWND hwnd = GetActiveWindow();

    BOOL dark = TRUE;

    // Windows 11 / newer Windows 10
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
}

void WinMessageBox(char* title, char* message, uint32_t type)
{
    MessageBoxA(NULL, message, title, type);
}

static CrashHandlerFn g_handler = NULL;

LONG WINAPI InternalCrashHandler(EXCEPTION_POINTERS* info)
{
    if (g_handler)
        g_handler(info);

    return EXCEPTION_EXECUTE_HANDLER;
}

void WinSetUnhandledExceptionFilter(CrashHandlerFn handler)
{
    g_handler = handler;
    SetUnhandledExceptionFilter(InternalCrashHandler);
}

#endif