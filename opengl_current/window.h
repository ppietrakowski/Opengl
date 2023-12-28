#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <memory>

#include "event.h"
#include "graphics_context.h"

struct WindowSettings
{
    int32_t Width;
    int32_t Height;
    std::string Title;
};

using EventCallback = std::function<void(const Event&)>;

class IWindow
{
public:
    virtual ~IWindow() = default;

    virtual void Update() = 0;

    virtual int32_t GetWidth() const = 0;
    virtual int32_t GetHeight() const = 0;

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
    virtual IGraphicsContext* GetContext() const = 0;

    virtual void Close() = 0;
    virtual void SetMouseVisible(bool bMouseVisible) = 0;

    static std::unique_ptr<IWindow> Create(const WindowSettings& window_settings);
};

