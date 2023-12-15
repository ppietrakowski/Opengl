#pragma once

#include <stdexcept>
#include <cstdint>

typedef struct ErrorHandlerInfo
{
    const char* FunctionName;
    const char* FileName;
    std::uint32_t Line;
    const char* ErrorMessage;
} ErrorHandlerInfo;

typedef struct SourceLocation
{
    const char* FileName;
    std::uint32_t Line;
    const char* FunctionName;
} SourceLocation;


#ifdef _MSC_VER
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

#ifdef _MSC_VER
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() __builtin_trap()
#endif

typedef void (*ErrorHandlerFn)(void* userData, const ErrorHandlerInfo* info);

typedef struct ErrorHandler
{
    ErrorHandlerFn ErrorHandlerFunc;
    void* UserData;
} ErrorHandler;

#ifdef __cplusplus
extern "C" {
#endif
    void AddErrorHandler(const ErrorHandler* handler);
    void RemoveErrorHandler(const ErrorHandler* handler);
    void Crash(const SourceLocation* location, const char* description);

    void PrintError(const SourceLocation* location, const char* message);
#ifdef __cplusplus
}
#endif

#define CURRENT_SOURCE_LOCATION {__FILE__, static_cast<std::uint32_t>(__LINE__), FUNCTION_SIGNATURE}

#define ERR_FAIL() SourceLocation location = CURRENT_SOURCE_LOCATION;  PrintError(&location, "Method/function failed"); return
#define ERR_FAIL_V(RetVal) SourceLocation location = CURRENT_SOURCE_LOCATION;  PrintError(&location, "Method/function failed"); return RetVal

#define ERR_FAIL_MSG(Msg) SourceLocation location = CURRENT_SOURCE_LOCATION;  PrintError(&location, Msg); return
#define ERR_FAIL_MSG_V(Msg, RetVal) SourceLocation location = CURRENT_SOURCE_LOCATION;  PrintError(&location, Msg); return RetVal

#define ERR_FAIL_NULL(Param) \
    if (Param == NULL) { ERR_FAIL_MSG(" Expression \"" #Param "\" evaluated to nullptr"); }

#define ERR_FAIL_NULL_MSG(Param, Msg) \
    if (Param == NULL) { ERR_FAIL_MSG(Msg); return; }

#define ERR_FAIL_NULL_V(Param, RetVal) \
    if (Param == NULL) { ERR_FAIL_MSG_V(" Expression \"" #Param "\" evaluated to nullptr", RetVal); }

#define ERR_FAIL_NULL_V_MSG(Param, RetVal, Msg) \
    if (Param == NULL) { ERR_FAIL_MSG_V(Msg, RetVal); }

#define ERR_FAIL_EXPECTED_FALSE(Condition) \
    if ((Condition)) { ERR_FAIL_MSG("Expression \"" #Condition "\" evaluated to true"); }

#define ERR_FAIL_EXPECTED_FALSE_V(Condition, RetVal) \
    if ((Condition)) { ERR_FAIL_MSG_V("Expression \"" #Condition "\" evaluated to true", RetVal); }

#define ERR_FAIL_EXPECTED_FALSE_MSG(Condition, Msg) \
    if ((Condition)) { ERR_FAIL_MSG(Msg); }

#define ERR_FAIL_EXPECTED_FALSE_V_MSG(Condition, Msg, RetVal) \
    if ((Condition)) { ERR_FAIL_MSG_V(Msg, RetVal); }

#define ERR_FAIL_EXPECTED_TRUE_MSG(Condition, Msg) \
    if (!(Condition)) { ERR_FAIL_MSG("Expression \"" #Condition "\" evaluated to false"); }

#ifdef ALLOW_MAYBE_CONTINUE_EXECUTION
#define MAYBE_MSG(Condition, Msg) if (!(Condition)) { const SourceLocation location = CURRENT_SOURCE_LOCATION;  PrintError(&location, Msg); }
#define MAYBE(Condition) MAYBE_MSG(Condition, "Condition " #Condition " is false")
#else
#define MAYBE_MSG(Condition, Msg) ERR_FAIL_EXPECTED_TRUE_MSG(Condition, Msg)
#define MAYBE(Condition) MAYBE_MSG(Condition, "Condition " #Condition " is false")
#endif

#define ERR_FAIL_EXPECTED_TRUE_V_MSG(Condition, Msg, RetVal) \
    if (!(Condition)) { ERR_FAIL_MSG_V(Msg, RetVal); }

#define ERR_FAIL_EXPECTED_TRUE(Condition) ERR_FAIL_EXPECTED_TRUE_MSG(Condition,  "Expression \"" #Condition "\" evaluated to false")

#define ERR_FAIL_EXPECTED_TRUE_V(Condition, RetVal) ERR_FAIL_EXPECTED_TRUE_V_MSG(Condition, "Expression \"" #Condition "\" evaluated to false", RetVal)

#define CRASH_EXPECTED_TRUE_MSG(Condition, Msg) \
    if (!(Condition)) { DEBUG_BREAK(); SourceLocation location = CURRENT_SOURCE_LOCATION; Crash(&location, Msg); }

#define CRASH_EXPECTED_TRUE(Condition) CRASH_EXPECTED_TRUE_MSG(Condition, "Fatal condition" #Condition " is false ")

#define CRASH_EXPECTED_FALSE_MSG(Condition, Msg) \
    if (Condition) { DEBUG_BREAK(); Crash(); }

#define CRASH_EXPECTED_FALSE(Condition) CRASH_EXPECTED_FALSE_MSG(Condition, "Fatal condition" #Condition " is true ")


#define DO_ONCE(Expression) { \
    static bool doneOnce = false; \
    \
    if (!doneOnce) { \
        do { Expression } while(0); \
        doneOnce=true; \
    } \
    } \

#define THROW_ERROR(Msg) throw std::runtime_error(Msg)

#if defined(_DEBUG) || defined(DEBUG)
#define ASSERT(Condition) if (!(Condition)) { SourceLocation location = CURRENT_SOURCE_LOCATION;  PrintError(&location, "Assertion " #Condition " evaluates to false"); DEBUG_BREAK(); THROW_ERROR("Fatal condition " #Condition " evaluates to false"); }
#else
#define ASSERT(Condition) ((void)0)
#endif