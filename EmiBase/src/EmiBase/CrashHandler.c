#include <stdio.h>

#include "EmiBase/CrashHandler.h"
#include "Libraries/WinAPI.h"

static const char* DecodeStatus(void* status)
{
    switch((size_t)status)
    {
        case 0:
            return "for an unknown reason.";
        case 1:
            return "while drawing EmiObjects.";
        case 2:
            return "during initialization.";
        case 3:
            return "while initializing Nuklear UI";
        case 4:
            return "while drawing Nuklear UI";
    }
    if((size_t)status < 255)
        return "for an unknown reason.";
    return (const char*)status;
}


static const char* ExceptionToString(DWORD code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION: return "Access Violation";
        case EXCEPTION_STACK_OVERFLOW: return "Stack Overflow";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal Instruction";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer Divide By Zero";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Float Divide By Zero";
        default: return "Unknown Exception";
    }
}


static void DecodeAccessViolation(EXCEPTION_RECORD* rec, const char** op, void** addr)
{
    ULONG_PTR type = rec->ExceptionInformation[0];
    *addr = (void*)rec->ExceptionInformation[1];

    if (type == 0) *op = "READ";
    else if (type == 1) *op = "WRITE";
    else if (type == 8) *op = "EXECUTE";
    else *op = "UNKNOWN";
}


/* single global status pointer */
void* _crashhandler_last_status = NULL;


#ifdef _WIN32

void WinCrashHandler(void* _info)
{
    EXCEPTION_POINTERS* info = (EXCEPTION_POINTERS*)_info;
    EXCEPTION_RECORD* rec = info->ExceptionRecord;

    const char* exception = ExceptionToString(rec->ExceptionCode);
    const char* status = DecodeStatus(_crashhandler_last_status);

    char av_block[256];
    const char* av_op = NULL;
    void* av_addr = NULL;
    const char* av_section = "";


    /* Only decode access violation info if relevant */
    if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        DecodeAccessViolation(rec, &av_op, &av_addr);

        snprintf(av_block, sizeof(av_block),
            "\n\nAccess Violation:\n"
            "  Operation: %s\n"
            "  Address: 0x%p\n\n",
            av_op,
            av_addr
        );

        av_section = av_block;
    }

    char buffer[2048];

    char* temp = "EmiBase crashed %s";
    if (_crashhandler_last_status != NULL && _crashhandler_last_status > (void*)255)
    {
        snprintf(buffer, sizeof(buffer),
            "Something went wrong while executing game code.\n\n"
            "Exception: %s (0x%08X)\n"
            "Fault Address: 0x%p\n\n"
            "Game crash while executing scene: '%s'"
            "%s",
            exception,
            rec->ExceptionCode,
            rec->ExceptionAddress,
            status,
            av_section
        );
    } else {
        snprintf(buffer, sizeof(buffer),
            "EmiBase unexpectedly crashed, we're sorry!\n\n"
            "Exception: %s (0x%08X)\n"
            "Fault Address: 0x%p\n\n"
            "Engine crash %s"
            "%s",
            exception,
            rec->ExceptionCode,
            rec->ExceptionAddress,
            status,
            av_section
        );
    }

    WinMessageBox("Fatal error!", buffer, MB_OK | MB_ICONERROR);
}

#endif


void CrashHandler_Init()
{
#ifdef _WIN32
    WinSetUnhandledExceptionFilter(WinCrashHandler);
#else
    // TODO: Linux
#endif
}


void _crashhandler_internal_sendstring(const char* status) {
    _crashhandler_last_status = (void*)status;
}
void _crashhandler_internal_sendstatus(size_t status) {
    _crashhandler_last_status = (void*)status;
}