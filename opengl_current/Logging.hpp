#pragma once

#include "ErrorMacros.hpp"

#include <memory>
#include "spdlog/spdlog.h"

class ILoggingContext
{
public:
    virtual ~ILoggingContext() = default;

    virtual std::shared_ptr<spdlog::logger> GetCoreLogger() const = 0;
    virtual std::shared_ptr<spdlog::logger> GetClientLogger() const = 0;
};

class Logging
{
public:
    static void Initialize();
    static void Quit();

    static std::shared_ptr<spdlog::logger> GetCoreLogger();
    static std::shared_ptr<spdlog::logger> GetClientLogger();

private:
    static std::unique_ptr<ILoggingContext> s_LoggerContext;
};

#define ENG_LOG(level, ...) SPDLOG_LOGGER_CALL(Logging::GetCoreLogger(), level, __VA_ARGS__)
#define ENG_LOG_INFO(Format, ...) ENG_LOG(spdlog::level::info, Format, __VA_ARGS__)
#define ENG_LOG_WARNING(Format, ...) ENG_LOG(spdlog::level::warn, Format, __VA_ARGS__)
#define ENG_LOG_ERROR(Format, ...) ENG_LOG(spdlog::level::err, Format, __VA_ARGS__)
#define ENG_LOG_DEBUG(Format, ...) ENG_LOG(spdlog::level::debug, Format, __VA_ARGS__)
#define ENG_LOG_VERBOSE(Format, ...) ENG_LOG(spdlog::level::trace, Format, __VA_ARGS__)
