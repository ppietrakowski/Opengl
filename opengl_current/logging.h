#pragma once

#include "error_macros.h"

#include <memory>

#define DECLARE_LOG_CATEGORY(LogCategoryName) struct LogCategory_##LogCategoryName { constexpr const char* GetName() const { return #LogCategoryName; } }; extern const LogCategory_##LogCategoryName LOG_##LogCategoryName
#define IMPLEMENT_LOG_CATEGORY(LogCategoryName) const LogCategory_##LogCategoryName LOG_##LogCategoryName

DECLARE_LOG_CATEGORY(CORE);
DECLARE_LOG_CATEGORY(RENDERER);
DECLARE_LOG_CATEGORY(ASSET_LOADING);
DECLARE_LOG_CATEGORY(GLOBAL);


#define ELOG(level, category, Format, ...) ::Logging::Log(LogInfo{CURRENT_SOURCE_LOCATION, category.GetName(), level, FormatString(Format ,__VA_ARGS__)})
#define ELOG_INFO(category, Format, ...) ELOG(LogLevel::kInfo, category, Format, __VA_ARGS__)
#define ELOG_WARNING(category, Format, ...) ELOG(LogLevel::kWarning, category, Format, __VA_ARGS__)
#define ELOG_ERROR(category, Format, ...) ELOG(LogLevel::kError, category, Format, __VA_ARGS__)
#define ELOG_DEBUG(category, Format, ...) ELOG(LogLevel::kDebug, category, Format, __VA_ARGS__)
#define ELOG_VERBOSE(category, Format, ...) ELOG(LogLevel::kVerbose, category, Format, __VA_ARGS__)

enum class LogLevel
{
    kInfo = 1,
    kWarning = 1 << 1,
    kError = 1 << 2,
    kDebug = 1 << 3,
    kVerbose = 1 << 4
};

struct LogInfo
{
    SourceLocation Location;
    const char* CategoryName;
    LogLevel Level;
    std::string Message;
};

class LogDevice
{
public:
    virtual ~LogDevice() = default;

public:
    virtual void Print(const LogInfo& info) = 0;
};

std::string FormatString(const char* format, ...);

using LogDeviceID = int16_t;

class Logging
{
public:
    static void Initialize();
    static void Quit();

    static void Log(const LogInfo& info);
    static void IgnoreLogLevel(LogLevel level);
    static void StopIgnoringLogLevel(LogLevel level);

    static LogDeviceID AddLogDevice(std::unique_ptr<LogDevice>&& device);
    static void RemoveLogDevice(LogDeviceID id);

    static void DisableStdLogging();
    static void EnableStdLogging();

private:
    static int32_t IgnoredLogLevels;

private:
    static void SortLogDeviceIDs();
};

