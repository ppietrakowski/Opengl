#include "error_macros.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <array>

#if defined(_WIN32) || defined(WIN32)     /* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows systems */
#include <Windows.h>
#endif

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
}