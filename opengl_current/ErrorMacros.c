#include "ErrorMacros.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_ERROR_HANDLERS 5
static ErrorHandler s_ErrorHandlers[MAX_ERROR_HANDLERS];
static uint16_t s_ErrorHandlersAssigned = 0;

void AddErrorHandler(const ErrorHandler* handler)
{
    DO_ONCE(
        memset(s_ErrorHandlers, 0, sizeof(s_ErrorHandlers));
    s_ErrorHandlersAssigned = 0;
    );

    ERR_FAIL_NULL(handler);
    ERR_FAIL_EXPECTED_TRUE_MSG(s_ErrorHandlersAssigned < MAX_ERROR_HANDLERS, "Max error handlers assigned");
    s_ErrorHandlers[s_ErrorHandlersAssigned++] = *handler;
}

void RemoveErrorHandler(const ErrorHandler* handler)
{
    ErrorHandler* lastIt = s_ErrorHandlers + MAX_ERROR_HANDLERS;
    ERR_FAIL_NULL(handler);

    for (ErrorHandler* it = s_ErrorHandlers; it != lastIt; ++it)
    {
        bool equal = handler->UserData == it->UserData && handler->ErrorHandlerFunc == it->ErrorHandlerFunc;
        if (equal)
        {
            for (ErrorHandler* moveIt = it + 1; moveIt != lastIt - 1; ++moveIt, ++it)
            {
                *it = *moveIt;
            }

            s_ErrorHandlersAssigned--;

            if (s_ErrorHandlersAssigned)
            {
                memset(s_ErrorHandlers, 0, sizeof(s_ErrorHandlers));
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
    ErrorHandlerInfo info;
    info.ErrorMessage = message;
    info.FileName = location->FileName;
    info.Line = location->Line;
    info.FunctionName = location->FunctionName;


    for (uint16_t i = 0; i < s_ErrorHandlersAssigned; ++i)
    {
        ErrorHandler* handler = &s_ErrorHandlers[i];
        handler->ErrorHandlerFunc(handler->UserData, &info);
    }
}
