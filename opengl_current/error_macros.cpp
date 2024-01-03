#include "error_macros.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <array>

#if defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
#include <Windows.h>
#endif

constexpr int32_t kMaxErrorHandlers = 5;

static std::array<ErrorHandler, kMaxErrorHandlers> ErrorHandlers;
static int32_t NumErrorHandlers = 0;

void AddErrorHandler(const ErrorHandler& handler)
{
    ERR_FAIL_EXPECTED_TRUE_MSG(NumErrorHandlers < kMaxErrorHandlers, "Max error handlers assigned");
    ErrorHandlers[NumErrorHandlers++] = handler;
}

void RemoveErrorHandler(const ErrorHandler& handler)
{
    // find first item that's equal to handler
    for (auto it = ErrorHandlers.begin(); it != ErrorHandlers.end(); ++it)
    {
        bool bHandlerEqual = handler.UserData == it->UserData && handler.ErrorHandlerFunc == it->ErrorHandlerFunc;

        if (bHandlerEqual)
        {
            // if found, move element to front of array
            std::move(it + 1, ErrorHandlers.end(), it);
            NumErrorHandlers--;
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

    ErrorHandlerInfo info{*location,  message};
    for (int32_t i = 0; i < NumErrorHandlers; ++i)
    {
        const ErrorHandler& errorHandler = ErrorHandlers[i];
        errorHandler.Invoke(info);
    }
}

void ErrorHandler::Invoke(const ErrorHandlerInfo& info) const
{
    ErrorHandlerFunc(UserData, info);
}
