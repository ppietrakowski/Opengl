#include "ErrorMacros.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <array>

#if defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
#include <Windows.h>
#endif

constexpr std::uint32_t MaxErrorHandlers = 5;

static std::array<ErrorHandler, MaxErrorHandlers> ErrorHandlers;
static std::uint32_t ErrorHandlersAssigned = 0;

void AddErrorHandler(const ErrorHandler& handler)
{
    ERR_FAIL_EXPECTED_TRUE_MSG(ErrorHandlersAssigned < MaxErrorHandlers, "Max error handlers assigned");
    ErrorHandlers[ErrorHandlersAssigned++] = handler;
}

void RemoveErrorHandler(const ErrorHandler& handler)
{
    // find first item that's equal to handler
    for (auto it = ErrorHandlers.begin(); it != ErrorHandlers.end(); ++it)
    {
        bool equal = handler.UserData == it->UserData && handler.ErrorHandlerFunc == it->ErrorHandlerFunc;

        if (equal)
        {
            // if found, move element to front of array
            std::move(it + 1, ErrorHandlers.end(), it);
            ErrorHandlersAssigned--;
            break;
        }
    }
}

void Crash(const SourceLocation* location, const char* description)
{
    PrintError(location, description);
#if defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
    MessageBoxA(nullptr, description, "Crash report", MB_OK);
#endif
    std::exit(EXIT_FAILURE);
}

void PrintError(const SourceLocation* location, const char* message)
{
#if defined(DEBUG) || defined(_DEBUG)
    printf("Error in %s: %u in %s msg: %s\n", location->FileName, location->Line, location->FunctionName, message);
#endif

    ErrorHandlerInfo info{ *location,  message };
    for (std::uint32_t i = 0; i < ErrorHandlersAssigned; ++i)
    {
        const ErrorHandler& errorHandler = ErrorHandlers[i];
        errorHandler.ErrorHandlerFunc(errorHandler.UserData, info);
    }
}
