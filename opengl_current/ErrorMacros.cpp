#include "ErrorMacros.hpp"

#include <iostream>

#if defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
#define ERROR_MACROS_WINDOW_DEFINED
#include <Windows.h>
#endif

void Crash(const SourceLocation& location, std::string_view description)
{
    PrintError(location, description);
#if defined(ERROR_MACROS_WINDOW_DEFINED) 
    std::string message(description.begin(), description.end());
    MessageBoxA(nullptr, message.c_str(), "Crash report", MB_OK);
#endif
    std::exit(EXIT_FAILURE);
}

void PrintError(const SourceLocation& location, std::string_view message)
{
#if defined(DEBUG) || defined(_DEBUG)
    std::clog << "Error in " << location.FileName;
    std::clog << ": " << location.Line << " in " << location.FunctionName;
    std::clog << "msg: " << message << std::endl;
#endif
}