#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <memory>

#include "event.h"
#include "graphics_context.h"

struct WindowSettings {
    uint32_t width;
    uint32_t height;
    std::string title;
};

using EventCallback = std::function<void(const Event&)>;

class Window {
public:
    virtual ~Window() = default;

    virtual void Update() = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual glm::ivec2 GetWindowPosition() const = 0;

    virtual glm::vec2 GetMousePosition() const = 0;
    virtual glm::vec2 GetLastMousePosition() const = 0;

    virtual bool IsOpen() const = 0;

    // Window attributes
    virtual void SetEventCallback(const EventCallback& callback) = 0;
    virtual void EnableVSync() = 0;
    virtual void DisableVSync() = 0;
    virtual bool IsVSyncEnabled() const = 0;

    virtual void* GetWindowNativeHandle() const = 0;
    virtual GraphicsContext* GetContext() const = 0;

    virtual void Close() = 0;
    virtual void SetMouseVisible(bool mouse_visible) = 0;

    static std::unique_ptr<Window> Create(const WindowSettings& window_settings);
};

