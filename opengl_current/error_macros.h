#pragma once

#include "core.h"
#include <cstdint>
#include <stdexcept>

struct SourceLocation
{
    std::string FileName;
    int32_t Line;
    std::string FunctionName;
};

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

void Crash(const SourceLocation& location, std::string_view description);
void PrintError(const SourceLocation& location, std::string_view message);

#define CURRENT_SOURCE_LOCATION                                       \
    SourceLocation {                                                                 \
        __FILE__, __LINE__, FUNCTION_SIGNATURE \
    }

#define ERR_FAIL()                                            \
    PrintError(CURRENT_SOURCE_LOCATION, "Method/function failed");   \
    return
#define ERR_FAIL_V(RetVal)                                    \
    PrintError(CURRENT_SOURCE_LOCATION, "Method/function failed");   \
    return RetVal

#define ERR_FAIL_MSG(Msg)                                     \
    PrintError(CURRENT_SOURCE_LOCATION, Msg);                        \
    return
#define ERR_FAIL_MSG_V(Msg, RetVal)                           \
    PrintError(CURRENT_SOURCE_LOCATION, Msg);                        \
    return (RetVal)

#define ERR_FAIL_NULL(Param)                                             \
    if (Param == nullptr) {                                              \
        ERR_FAIL_MSG(" Expression \"" #Param "\" evaluated to nullptr"); \
    }

#define ERR_FAIL_NULL_MSG(Param, Msg) \
    if (Param == nullptr) {           \
        ERR_FAIL_MSG(Msg);            \
        return;                       \
    }

#define ERR_FAIL_NULL_V(Param, RetVal)                                             \
    if (Param == nullptr) {                                                        \
        ERR_FAIL_MSG_V(" Expression \"" #Param "\" evaluated to nullptr", RetVal); \
    }

#define ERR_FAIL_NULL_V_MSG(Param, RetVal, Msg) \
    if (Param == nullptr) {                     \
        ERR_FAIL_MSG_V(Msg, RetVal);            \
    }

#define ERR_FAIL_EXPECTED_FALSE(Condition)                               \
    if ((Condition)) {                                                   \
        ERR_FAIL_MSG("Expression \"" #Condition "\" evaluated to true"); \
    }

#define ERR_FAIL_EXPECTED_FALSE_V(Condition, RetVal)                               \
    if ((Condition)) {                                                             \
        ERR_FAIL_MSG_V("Expression \"" #Condition "\" evaluated to true", RetVal); \
    }

#define ERR_FAIL_EXPECTED_FALSE_MSG(Condition, Msg) \
    if ((Condition)) {                              \
        ERR_FAIL_MSG(Msg);                          \
    }

#define ERR_FAIL_EXPECTED_FALSE_V_MSG(Condition, Msg, RetVal) \
    if ((Condition)) {                                        \
        ERR_FAIL_MSG_V(Msg, RetVal);                          \
    }

#define ERR_FAIL_EXPECTED_TRUE_MSG(Condition, Msg)                        \
    if (!(Condition)) {                                                   \
        ERR_FAIL_MSG("Expression \"" #Condition "\" evaluated to false"); \
    }

#ifdef ALLOW_MAYBE_CONTINUE_EXECUTION
#define MAYBE_MSG(Condition, Msg)                                \
    if (!(Condition)) {                                          \
        PrintError(CURRENT_SOURCE_LOCATION, Msg);                              \
    }
#define MAYBE(Condition) MAYBE_MSG(Condition, "Condition " #Condition " is false")
#else
#define MAYBE_MSG(Condition, Msg) ERR_FAIL_EXPECTED_TRUE_MSG(Condition, Msg)
#define MAYBE(Condition) MAYBE_MSG(Condition, "Condition " #Condition " is false")
#endif

#define ERR_FAIL_EXPECTED_TRUE_V_MSG(Condition, Msg, RetVal) \
    if (!(Condition)) {                                      \
        ERR_FAIL_MSG_V(Msg, RetVal);                         \
    }

#define ERR_FAIL_EXPECTED_TRUE(Condition) ERR_FAIL_EXPECTED_TRUE_MSG(Condition, "Expression \"" #Condition "\" evaluated to false")

#define ERR_FAIL_EXPECTED_TRUE_V(Condition, RetVal) ERR_FAIL_EXPECTED_TRUE_V_MSG(Condition, "Expression \"" #Condition "\" evaluated to false", RetVal)

#define CRASH_EXPECTED_TRUE_MSG(Condition, Msg)                   \
    if (!(Condition)) {                                           \
        DEBUG_BREAK();                                            \
        Crash(CURRENT_SOURCE_LOCATION, Msg);                             \
    }

#define CRASH_EXPECTED_TRUE(Condition) CRASH_EXPECTED_TRUE_MSG(Condition, "Fatal condition \"" #Condition "\" is false ")
#define CRASH_EXPECTED_NOT_NULL(Obj) CRASH_EXPECTED_TRUE_MSG(Obj != nullptr, "Fatal condition \"" #Obj "\" is nullptr ")

#define CRASH_EXPECTED_FALSE_MSG(Condition, Msg) \
    if (Condition) {                             \
        DEBUG_BREAK();                           \
        Crash(CURRENT_SOURCE_LOCATION, Msg);                                 \
    }

#define CRASH_EXPECTED_FALSE(Condition) CRASH_EXPECTED_FALSE_MSG(Condition, "Fatal condition \"" #Condition "\" is true ")

#define DO_ONCE(Lambda)            \
    {                                  \
        static bool bDoneOnce = false; \
                                       \
        if (!bDoneOnce) {              \
            do {                       \
                Lambda();              \
            } while (0);               \
            bDoneOnce = true;          \
        }                              \
    }

#define THROW_ERROR(Msg) throw std::runtime_error(Msg)

#if defined(_DEBUG) || defined(DEBUG)
#define ASSERT(Condition)                                                            \
    if (!(Condition)) {                                                              \
        PrintError(CURRENT_SOURCE_LOCATION, "Assertion " #Condition " evaluates to false"); \
        DEBUG_BREAK();                                                               \
        THROW_ERROR("Fatal condition " #Condition " evaluates to false");            \
    }
#else
#define ASSERT(Condition) ((void)0)
#endif