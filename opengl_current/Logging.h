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
#define ELOG_INFO(category, Format, ...) ELOG(ELogLevel::Info, category, Format, __VA_ARGS__)
#define ELOG_WARNING(category, Format, ...) ELOG(ELogLevel::Warning, category, Format, __VA_ARGS__)
#define ELOG_ERROR(category, Format, ...) ELOG(ELogLevel::Error, category, Format, __VA_ARGS__)
#define ELOG_DEBUG(category, Format, ...) ELOG(ELogLevel::Debug, category, Format, __VA_ARGS__)
#define ELOG_VERBOSE(category, Format, ...) ELOG(ELogLevel::Verbose, category, Format, __VA_ARGS__)

enum class ELogLevel : unsigned int
{
    Info = 1,
    Warning = 1 << 1,
    Error = 1 << 2,
    Debug = 1 << 3,
    Verbose = 1 << 4
};

class ILogDevice
{
public:
    virtual ~ILogDevice() = default;

public:
    virtual void Print(const SourceLocation& location, const char* categoryName, ELogLevel logLevel, const std::string& format) = 0;
};

std::string FormatString(const char* format, ...);

typedef short LogDeviceID;

class Logging
{
public:
    static void Initialize();
    static void Quit();

    static void Log(const SourceLocation& location, const char* categoryName, ELogLevel logLevel, const std::string& format);
    static void IgnoreLogLevel(ELogLevel level);
    static void StopIgnoringLogLevel(ELogLevel level);

    static LogDeviceID AddLogDevice(std::unique_ptr<ILogDevice>&& device);
    static void RemoveLogDevice(LogDeviceID id);

    static void DisableStdLogging();
    static void EnableStdLogging();

private:
    static unsigned int IgnoredLogLevels;

private:
    static void SortLogDeviceIDs();
};

