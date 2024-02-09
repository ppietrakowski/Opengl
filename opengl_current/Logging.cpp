#include "Logging.hpp"

#include <iostream>
#include <cstdarg>
#include <algorithm>
#include <vector>

#include <ctime>
#include <iomanip>
#include "spdlog/sinks/stdout_color_sinks.h"

class LoggingContext : public ILoggingContext
{
public:
    LoggingContext()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
    
        m_CoreLogger = spdlog::stdout_color_mt("Core");
        m_ClientLogger = spdlog::stdout_color_mt("Client");

        m_CoreLogger->set_level(spdlog::level::trace);
        m_ClientLogger->set_level(spdlog::level::trace);
    }

    // Inherited via ILoggingContext
    std::shared_ptr<spdlog::logger> GetCoreLogger() const override
    {
        return m_CoreLogger;
    }
    std::shared_ptr<spdlog::logger> GetClientLogger() const override
    {
        return m_ClientLogger;
    }

private:
    std::shared_ptr<spdlog::logger> m_CoreLogger;
    std::shared_ptr<spdlog::logger> m_ClientLogger;
};

std::unique_ptr<ILoggingContext> Logging::s_LoggerContext;

void Logging::Initialize()
{
    s_LoggerContext.reset(new LoggingContext());
}

void Logging::Quit()
{
    s_LoggerContext.reset();
}

std::shared_ptr<spdlog::logger> Logging::GetCoreLogger()
{
    return s_LoggerContext->GetCoreLogger();
}

std::shared_ptr<spdlog::logger> Logging::GetClientLogger()
{
    return s_LoggerContext->GetClientLogger();
}