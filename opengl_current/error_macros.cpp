#include "error_macros.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <array>

#if defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
#include <Windows.h>
#endif

constexpr size_t kMaxErrorHandlers = 5;

static std::array<ErrorHandler, kMaxErrorHandlers> error_handlers_;
static size_t num_error_handlers_ = 0;

void AddErrorHandler(const ErrorHandler& handler)
{
    ERR_FAIL_EXPECTED_TRUE_MSG(num_error_handlers_ < kMaxErrorHandlers, "Max error handlers assigned");
    error_handlers_[num_error_handlers_++] = handler;
}

void RemoveErrorHandler(const ErrorHandler& handler)
{
    // find first item that's equal to handler
    for (auto it = error_handlers_.begin(); it != error_handlers_.end(); ++it)
    {
        bool handler_equal = handler.user_data == it->user_data && handler.error_handler_func == it->error_handler_func;

        if (handler_equal)
        {
            // if found, move element to front of array
            std::move(it + 1, error_handlers_.end(), it);
            num_error_handlers_--;
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
    printf("Error in %s: %u in %s msg: %s\n", location->file_name, location->line, location->function_name, message);
#endif

    ErrorHandlerInfo info{*location,  message};
    for (size_t i = 0; i < num_error_handlers_; ++i)
    {
        const ErrorHandler& error_handler = error_handlers_[i];
        error_handler.Invoke(info);
    }
}

void ErrorHandler::Invoke(const ErrorHandlerInfo& info) const
{
    error_handler_func(user_data, info);
}
