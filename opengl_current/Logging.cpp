#include "Logging.h"

#include <iostream>
#include <cstdarg>
#include <algorithm>

#define STD_LOGGER_ID 0

IMPLEMENT_LOG_CATEGORY(CORE);
IMPLEMENT_LOG_CATEGORY(RENDERER);
IMPLEMENT_LOG_CATEGORY(ASSET_LOADING);
IMPLEMENT_LOG_CATEGORY(GLOBAL);

unsigned int Logging::IgnoredLogLevels = 0;

struct Device
{
    std::unique_ptr<ILogDevice> Log;
    LogDeviceID ID;

    ILogDevice* operator->() const { return Log.get(); }
};

static std::vector<Device> LogDevices;
static LogDeviceID LastID = 1;

namespace
{
#define CASE(x) case ELogLevel::##x: return #x
    const char* ToString(ELogLevel logLevel)
    {
        switch (logLevel)
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

class StdLogDevice : public ILogDevice
{
    void Print(const SourceLocation& location, const char* categoryName, ELogLevel logLevel, const std::string& format) override
    {
        std::clog << "[" << categoryName << " " << ToString(logLevel) << "] \"" <<
            format << "\" in " << location.FunctionName << " in " << location.FileName << ": " << location.Line << std::endl << std::endl;
    }
};

void Logging::Initialize()
{
    LogDevices.push_back(Device{ std::make_unique<StdLogDevice>(), STD_LOGGER_ID });
}

void Logging::IgnoreLogLevel(ELogLevel level)
{
    IgnoredLogLevels = IgnoredLogLevels | static_cast<unsigned int>(level);
}

void Logging::StopIgnoringLogLevel(ELogLevel level)
{
    IgnoredLogLevels = IgnoredLogLevels & (~static_cast<unsigned int>(level));
}

void Logging::Quit()
{
    LogDevices.clear();
}

void Logging::Log(const SourceLocation& location, const char* categoryName, ELogLevel logLevel, const std::string& format)
{
    if (IgnoredLogLevels & static_cast<unsigned int>(logLevel))
    {
        return;
    }

    for (const Device& device : LogDevices)
    {
        device->Print(location, categoryName, logLevel, format);
    }
}

LogDeviceID Logging::AddLogDevice(std::unique_ptr<ILogDevice>&& device)
{
    LogDeviceID ID = LastID++;
    LogDevices.push_back(Device{ std::move(device), ID });
    SortLogDeviceIDs();
    return ID;
}

void Logging::RemoveLogDevice(LogDeviceID id)
{
    auto it = std::find_if(LogDevices.begin(), LogDevices.end(), [id](const Device& device)
    {
        return device.ID == id;
    });

    if (it != LogDevices.end())
    {
        LogDevices.erase(it);
    }
}

void Logging::DisableStdLogging()
{
    RemoveLogDevice(STD_LOGGER_ID);
}

void Logging::EnableStdLogging()
{
    LogDevices.push_back(Device{ std::make_unique<StdLogDevice>(), STD_LOGGER_ID});
    SortLogDeviceIDs();
}

void Logging::SortLogDeviceIDs()
{
    std::sort(LogDevices.begin(), LogDevices.end(), [](const Device& a, const Device& b) { return a.ID < b.ID; });
}

std::string FormatString(const char* format, ...)
{
    std::vector<char> buffer(16348, 0);

    va_list list;

    va_start(list, format);
    int sprintfSize = vsnprintf(buffer.data(), buffer.size(), format, list);
    va_end(list);

    if (sprintfSize < 0)
    {
        return "";
    }
    else if (sprintfSize == 0)
    {
        return buffer.data();
    }

    return std::string{ buffer.data(), buffer.data() + sprintfSize };
}
