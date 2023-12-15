#include "ErrorMacros.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

constexpr std::uint32_t MaxErrorHandlers = 5;

static ErrorHandler ErrorHandlers[MaxErrorHandlers];
static std::uint32_t ErrorHandlersAssigned = 0;

void AddErrorHandler(const ErrorHandler* handler)
{
    DO_ONCE(
        memset(ErrorHandlers, 0, sizeof(ErrorHandlers));
    ErrorHandlersAssigned = 0;
    );

    ERR_FAIL_NULL(handler);
    ERR_FAIL_EXPECTED_TRUE_MSG(ErrorHandlersAssigned < MaxErrorHandlers, "Max error handlers assigned");
    ErrorHandlers[ErrorHandlersAssigned++] = *handler;
}

void RemoveErrorHandler(const ErrorHandler* handler)
{
    ErrorHandler* lastIt = ErrorHandlers + MaxErrorHandlers;
    ERR_FAIL_NULL(handler);

    for (ErrorHandler* it = ErrorHandlers; it != lastIt; ++it)
    {
        bool equal = handler->UserData == it->UserData && handler->ErrorHandlerFunc == it->ErrorHandlerFunc;
        if (equal)
        {
            for (ErrorHandler* moveIt = it + 1; moveIt != lastIt - 1; ++moveIt, ++it)
            {
                *it = *moveIt;
            }

            ErrorHandlersAssigned--;

            if (ErrorHandlersAssigned)
            {
                memset(ErrorHandlers, 0, sizeof(ErrorHandlers));
            }

            break;
        }
    }
}

void Crash(const SourceLocation* location, const char* description)
{
    PrintError(location, description);
    exit(EXIT_FAILURE);
}

void PrintError(const SourceLocation* location, const char* message)
{
#if defined(DEBUG) || defined(_DEBUG)
    printf("Error in %s: %u in %s msg: %s\n", location->FileName, location->Line, location->FunctionName, message);
#endif

    ErrorHandlerInfo info{};
    info.ErrorMessage = message;
    info.FileName = location->FileName;
    info.Line = location->Line;
    info.FunctionName = location->FunctionName;

    for (std::uint32_t i = 0; i < ErrorHandlersAssigned; ++i)
    {
        ErrorHandler* handler = &ErrorHandlers[i];
        handler->ErrorHandlerFunc(handler->UserData, &info);
    }
}
