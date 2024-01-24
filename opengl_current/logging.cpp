#include "logging.h"

#include <iostream>
#include <cstdarg>
#include <algorithm>
#include <vector>

static constexpr LogDeviceID StdLoggerId = 0;

IMPLEMENT_LOG_CATEGORY(CORE);
IMPLEMENT_LOG_CATEGORY(RENDERER);
IMPLEMENT_LOG_CATEGORY(ASSET_LOADING);
IMPLEMENT_LOG_CATEGORY(GLOBAL);

int Logging::s_IgnoredLogLevels = 0;

struct Device
{
    std::unique_ptr<LogDevice> Device;
    LogDeviceID DeviceId;

    LogDevice* operator->() const
    {
        return Device.get();
    }
};

static std::vector<Device> LogDevices;
static LogDeviceID LastLogDeviceId = 1;

namespace
{
#define CASE(x) case LogLevel::##x: return #x
    const char* ToString(LogLevel log_level)
    {
        switch (log_level)
        {
            CASE(Info);
            CASE(Warning);
            CASE(Error);
            CASE(Debug);
            CASE(Verbose);
        }

        return "Invalid log level";
    }
#undef CASE
}

class StdLogDevice : public LogDevice
{
public:
    StdLogDevice(std::ostream& logger) :
        m_Logger{&logger}
    {
    }

    void Print(const LogInfo& info) override
    {
        const SourceLocation& location = info.CodeLocation;

        *m_Logger << "[" << info.CategoryName << " " << ToString(info.Level) << "] \"" <<
            info.Message << "\" in " << location.FunctionName << " in " << location.FileName << ": " << location.Line << std::endl << std::endl;
    }

private:
    std::ostream* m_Logger;
};

void Logging::Initialize()
{
    EnableStdLogging();
}

void Logging::IgnoreLogLevel(LogLevel level)
{
    s_IgnoredLogLevels = s_IgnoredLogLevels | static_cast<int>(level);
}

void Logging::StopIgnoringLogLevel(LogLevel level)
{
    s_IgnoredLogLevels = s_IgnoredLogLevels & (~static_cast<int>(level));
}

void Logging::Quit()
{
    LogDevices.clear();
}

void Logging::Log(const LogInfo& info)
{
    if (s_IgnoredLogLevels & static_cast<int>(info.Level))
    {
        return;
    }

    for (const Device& device : LogDevices)
    {
        device->Print(info);
    }
}

LogDeviceID Logging::AddLogDevice(std::unique_ptr<LogDevice>&& device)
{
    LogDeviceID deviceId = LastLogDeviceId++;
    LogDevices.emplace_back(Device{std::move(device), deviceId});
    SortLogDeviceIDs();
    return deviceId;
}

void Logging::RemoveLogDevice(LogDeviceID id)
{
    auto it = std::find_if(LogDevices.begin(), LogDevices.end(), [id](const Device& device)
    {
        return device.DeviceId == id;
    });

    if (it != LogDevices.end())
    {
        LogDevices.erase(it);
    }
}

void Logging::DisableStdLogging()
{
    RemoveLogDevice(StdLoggerId);
}

void Logging::EnableStdLogging()
{
    LogDevices.emplace_back(Device{CreateStdStreamLogger(std::clog), StdLoggerId});
    SortLogDeviceIDs();
}

void Logging::SortLogDeviceIDs()
{
    std::sort(LogDevices.begin(), LogDevices.end(), [](const Device& a, const Device& b)
    {
        return a.DeviceId < b.DeviceId;
    });
}

std::string FormatString(const char* format, ...)
{
    std::vector<char> buffer(16348, 0);

    va_list list;

    va_start(list, format);
    int bufferLength = vsnprintf(buffer.data(), buffer.size(), format, list);
    va_end(list);

    if (bufferLength < 0)
    {
        return "";
    }
    else if (bufferLength == 0)
    {
        return buffer.data();
    }

    return std::string{buffer.data(), buffer.data() + bufferLength};
}

std::unique_ptr<LogDevice> CreateStdStreamLogger(std::ostream& stream)
{
    return std::make_unique<StdLogDevice>(stream);
}
