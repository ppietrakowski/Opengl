#include "Logging.h"

#include <iostream>
#include <cstdarg>
#include <algorithm>

static constexpr LogDeviceID kStdLoggerId = 0;

IMPLEMENT_LOG_CATEGORY(CORE);
IMPLEMENT_LOG_CATEGORY(RENDERER);
IMPLEMENT_LOG_CATEGORY(ASSET_LOADING);
IMPLEMENT_LOG_CATEGORY(GLOBAL);

uint32_t Logging::ignored_log_levels_ = 0;

struct Device {
    std::unique_ptr<LogDevice> log_device;
    LogDeviceID device_id;

    LogDevice* operator->() const {
        return log_device.get();
    }
};

static std::vector<Device> log_devices_;
static LogDeviceID last_log_device_id_ = 1;

namespace {
#define CASE(x) case LogLevel::##x: return #x
    const char* ToString(LogLevel log_level) {
        switch (log_level) {
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

class StdLogDevice : public LogDevice {
    void Print(const SourceLocation& location, const char* category_name, LogLevel log_level, const std::string& format) override {
        std::clog << "[" << category_name << " " << ToString(log_level) << "] \"" <<
            format << "\" in " << location.function_name << " in " << location.file_name << ": " << location.line << std::endl << std::endl;
    }
};

void Logging::Initialize() {
    log_devices_.emplace_back(Device{ std::make_unique<StdLogDevice>(), kStdLoggerId });
}

void Logging::IgnoreLogLevel(LogLevel level) {
    ignored_log_levels_ = ignored_log_levels_ | static_cast<uint32_t>(level);
}

void Logging::StopIgnoringLogLevel(LogLevel level) {
    ignored_log_levels_ = ignored_log_levels_ & (~static_cast<uint32_t>(level));
}

void Logging::Quit() {
    log_devices_.clear();
}

void Logging::Log(const SourceLocation& location, const char* categoryName, LogLevel logLevel, const std::string& format) {
    if (ignored_log_levels_ & static_cast<uint32_t>(logLevel)) {
        return;
    }

    for (const Device& device : log_devices_) {
        device->Print(location, categoryName, logLevel, format);
    }
}

LogDeviceID Logging::AddLogDevice(std::unique_ptr<LogDevice>&& device) {
    LogDeviceID device_id = last_log_device_id_++;
    log_devices_.emplace_back(Device{ std::move(device), device_id });
    SortLogDeviceIDs();
    return device_id;
}

void Logging::RemoveLogDevice(LogDeviceID id) {
    auto it = std::find_if(log_devices_.begin(), log_devices_.end(), [id](const Device& device) {
        return device.device_id == id;
    });

    if (it != log_devices_.end()) {
        log_devices_.erase(it);
    }
}

void Logging::DisableStdLogging() {
    RemoveLogDevice(kStdLoggerId);
}

void Logging::EnableStdLogging() {
    log_devices_.emplace_back(Device{ std::make_unique<StdLogDevice>(), kStdLoggerId });
    SortLogDeviceIDs();
}

void Logging::SortLogDeviceIDs() {
    std::sort(log_devices_.begin(), log_devices_.end(), [](const Device& a, const Device& b) { return a.device_id < b.device_id; });
}

std::string FormatString(const char* format, ...) {
    std::vector<char> buffer(16348, 0);

    va_list list;

    va_start(list, format);
    int32_t sprinf_size = vsnprintf(buffer.data(), buffer.size(), format, list);
    va_end(list);

    if (sprinf_size < 0) {
        return "";
    } else if (sprinf_size == 0) {
        return buffer.data();
    }

    return std::string{ buffer.data(), buffer.data() + sprinf_size };
}
