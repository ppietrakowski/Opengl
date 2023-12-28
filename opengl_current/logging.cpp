#include "logging.h"

#include <iostream>
#include <cstdarg>
#include <algorithm>
#include <vector>

static constexpr LogDeviceID kStdLoggerId = 0;

IMPLEMENT_LOG_CATEGORY(CORE);
IMPLEMENT_LOG_CATEGORY(RENDERER);
IMPLEMENT_LOG_CATEGORY(ASSET_LOADING);
IMPLEMENT_LOG_CATEGORY(GLOBAL);

int32_t Logging::s_IgnoredLogLevels = 0;

struct Device
{
    std::unique_ptr<LogDevice> Log;
    LogDeviceID DeviceId;

    LogDevice* operator->() const
    {
        return Log.get();
    }
};

static std::vector<Device> s_LogDevices;
static LogDeviceID s_LastLogDeviceId = 1;

namespace
{
#define CASE(x) case LogLevel::##x: return #x
    const char* ToString(LogLevel logLevel)
    {
        switch (logLevel)
        {
            CASE(kInfo);
            CASE(kWarning);
            CASE(kError);
            CASE(kDebug);
            CASE(kVerbose);
        }

        return "Invalid log level";
    }
#undef CASE
}

class StdLogDevice : public LogDevice
{
    void Print(const LogInfo& info) override
    {
        const SourceLocation& location = info.Location;

        std::clog << "[" << info.CategoryName << " " << ToString(info.Level) << "] \"" <<
            info.Message << "\" in " << location.FunctionName << " in " << location.FileName << ": " << location.Line << std::endl << std::endl;
    }
};

void Logging::Initialize()
{
    s_LogDevices.emplace_back(Device{std::make_unique<StdLogDevice>(), kStdLoggerId});
}

void Logging::IgnoreLogLevel(LogLevel level)
{
    s_IgnoredLogLevels = s_IgnoredLogLevels | static_cast<uint32_t>(level);
}

void Logging::StopIgnoringLogLevel(LogLevel level)
{
    s_IgnoredLogLevels = s_IgnoredLogLevels & (~static_cast<uint32_t>(level));
}

void Logging::Quit()
{
    s_LogDevices.clear();
}

void Logging::Log(const LogInfo& info)
{
    if (s_IgnoredLogLevels & static_cast<int32_t>(info.Level))
    {
        return;
    }

    for (const Device& device : s_LogDevices)
    {
        device->Print(info);
    }
}

LogDeviceID Logging::AddLogDevice(std::unique_ptr<LogDevice>&& device)
{
    LogDeviceID deviceId = s_LastLogDeviceId++;
    s_LogDevices.emplace_back(Device{std::move(device), deviceId});
    SortLogDeviceIDs();
    return deviceId;
}

void Logging::RemoveLogDevice(LogDeviceID id)
{
    auto it = std::find_if(s_LogDevices.begin(), s_LogDevices.end(), [id](const Device& device) {
        return device.DeviceId == id;
    });

    if (it != s_LogDevices.end())
    {
        s_LogDevices.erase(it);
    }
}

void Logging::DisableStdLogging()
{
    RemoveLogDevice(kStdLoggerId);
}

void Logging::EnableStdLogging()
{
    s_LogDevices.emplace_back(Device{std::make_unique<StdLogDevice>(), kStdLoggerId});
    SortLogDeviceIDs();
}

void Logging::SortLogDeviceIDs()
{
    std::sort(s_LogDevices.begin(), s_LogDevices.end(), [](const Device& a, const Device& b) { return a.DeviceId < b.DeviceId; });
}

std::string FormatString(const char* format, ...)
{
    std::vector<char> buffer(16348, 0);

    va_list list;

    va_start(list, format);
    int32_t numCharsWritten = vsnprintf(buffer.data(), buffer.size(), format, list);
    va_end(list);

    if (numCharsWritten < 0)
    {
        return "";
    }
    else if (numCharsWritten == 0)
    {
        return buffer.data();
    }

    return std::string{buffer.data(), buffer.data() + numCharsWritten};
}
