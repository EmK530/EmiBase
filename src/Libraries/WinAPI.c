#ifdef _WIN32

#include <stdint.h>
#include "Libraries/WinAPI.h"

void SetDarkTitleBar(void)
{
    HWND hwnd = GetActiveWindow();

    BOOL dark = 1;

    // Windows 11 / newer Windows 10
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
}

void WinMessageBox(char* title, char* message, uint32_t type)
{
    MessageBoxA(NULL, message, title, type);
}

static CrashHandlerFn g_handler = NULL;

LONG __stdcall InternalCrashHandler(EXCEPTION_POINTERS* info)
{
    if (g_handler)
        g_handler(info);

    return 1;
}

void WinSetUnhandledExceptionFilter(CrashHandlerFn handler)
{
    g_handler = handler;
    SetUnhandledExceptionFilter(InternalCrashHandler);
}

#endif