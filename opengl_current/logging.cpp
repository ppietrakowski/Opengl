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

int Logging::ignored_log_levels_ = 0;

struct Device
{
    std::unique_ptr<LogDevice> device;
    LogDeviceID device_id;

    LogDevice* operator->() const
    {
        return device.get();
    }
};

static std::vector<Device> log_devices_;
static LogDeviceID last_log_device_id_ = 1;

namespace
{
#define CASE(x) case LogLevel::k##x: return #x
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
        const SourceLocation& location = info.source_location;

        *m_Logger << "[" << info.category_name << " " << ToString(info.log_level) << "] \"" <<
            info.message << "\" in " << location.function_name << " in " << location.file_name << ": " << location.line << std::endl << std::endl;
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
    ignored_log_levels_ = ignored_log_levels_ | static_cast<int>(level);
}

void Logging::StopIgnoringLogLevel(LogLevel level)
{
    ignored_log_levels_ = ignored_log_levels_ & (~static_cast<int>(level));
}

void Logging::Quit()
{
    log_devices_.clear();
}

void Logging::Log(const LogInfo& info)
{
    if (ignored_log_levels_ & static_cast<std::int32_t>(info.log_level))
    {
        return;
    }

    for (const Device& device : log_devices_)
    {
        device->Print(info);
    }
}

LogDeviceID Logging::AddLogDevice(std::unique_ptr<LogDevice>&& device)
{
    LogDeviceID device_id = last_log_device_id_++;
    log_devices_.emplace_back(Device{std::move(device), device_id});
    SortLogDeviceIDs();
    return device_id;
}

void Logging::RemoveLogDevice(LogDeviceID id)
{
    auto it = std::find_if(log_devices_.begin(), log_devices_.end(), [id](const Device& device) {
        return device.device_id == id;
    });

    if (it != log_devices_.end())
    {
        log_devices_.erase(it);
    }
}

void Logging::DisableStdLogging()
{
    RemoveLogDevice(kStdLoggerId);
}

void Logging::EnableStdLogging()
{
    log_devices_.emplace_back(Device{CreateStdStreamLogger(std::clog), kStdLoggerId});
    SortLogDeviceIDs();
}

void Logging::SortLogDeviceIDs()
{
    std::sort(log_devices_.begin(), log_devices_.end(), [](const Device& a, const Device& b) { return a.device_id < b.device_id; });
}

std::string FormatString(const char* format, ...)
{
    std::vector<char> buffer(16348, 0);

    va_list list;

    va_start(list, format);
    int buffer_length = vsnprintf(buffer.data(), buffer.size(), format, list);
    va_end(list);

    if (buffer_length < 0)
    {
        return "";
    } else if (buffer_length == 0)
    {
        return buffer.data();
    }

    return std::string{buffer.data(), buffer.data() + buffer_length};
}

std::unique_ptr<LogDevice> CreateStdStreamLogger(std::ostream& stream)
{
    return std::make_unique<StdLogDevice>(stream);
}
