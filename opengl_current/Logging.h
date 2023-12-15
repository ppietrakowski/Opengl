#pragma once

#include "ErrorMacros.h"

#include <memory>
#include <vector>

#define DECLARE_LOG_CATEGORY(LogCategoryName) struct LogCategory_##LogCategoryName { constexpr const char* GetName() const { return #LogCategoryName; } }; extern const LogCategory_##LogCategoryName LOG_##LogCategoryName
#define IMPLEMENT_LOG_CATEGORY(LogCategoryName) const LogCategory_##LogCategoryName LOG_##LogCategoryName

DECLARE_LOG_CATEGORY(CORE);
DECLARE_LOG_CATEGORY(RENDERER);
DECLARE_LOG_CATEGORY(ASSET_LOADING);
DECLARE_LOG_CATEGORY(GLOBAL);


#define ELOG(level, category, Format, ...) ::Logging::Log(CURRENT_SOURCE_LOCATION, category.GetName(), level, FormatString(Format ,__VA_ARGS__))
#define ELOG_INFO(category, Format, ...) ELOG(LogLevel::Info, category, Format, __VA_ARGS__)
#define ELOG_WARNING(category, Format, ...) ELOG(LogLevel::Warning, category, Format, __VA_ARGS__)
#define ELOG_ERROR(category, Format, ...) ELOG(LogLevel::Error, category, Format, __VA_ARGS__)
#define ELOG_DEBUG(category, Format, ...) ELOG(LogLevel::Debug, category, Format, __VA_ARGS__)
#define ELOG_VERBOSE(category, Format, ...) ELOG(LogLevel::Verbose, category, Format, __VA_ARGS__)

enum class LogLevel : std::uint32_t
{
    Info = 1,
    Warning = 1 << 1,
    Error = 1 << 2,
    Debug = 1 << 3,
    Verbose = 1 << 4
};

class LogDevice
{
public:
    virtual ~LogDevice() = default;

public:
    virtual void Print(const SourceLocation& location, const char* categoryName, LogLevel logLevel, const std::string& format) = 0;
};

std::string FormatString(const char* format, ...);

using LogDeviceID = std::int16_t;

class Logging
{
public:
    static void Initialize();
    static void Quit();

    static void Log(const SourceLocation& location, const char* categoryName, LogLevel logLevel, const std::string& format);
    static void IgnoreLogLevel(LogLevel level);
    static void StopIgnoringLogLevel(LogLevel level);

    static LogDeviceID AddLogDevice(std::unique_ptr<LogDevice>&& device);
    static void RemoveLogDevice(LogDeviceID id);

    static void DisableStdLogging();
    static void EnableStdLogging();

private:
    static std::uint32_t IgnoredLogLevels;

private:
    static void SortLogDeviceIDs();
};

