#pragma once

#include "event.h"

#include <GL/glew.h>
#include <typeindex>
#include <chrono>

class IPlatform {
public:
    IPlatform() {
        instance_ = this;
    }
    virtual ~IPlatform() = default;

public:
    virtual glm::vec2 GetMousePosition() const = 0;
    virtual glm::vec2 GetLastMousePosition() const = 0;

    virtual void SetMouseVisible(bool mouse_visible) = 0;

    virtual bool IsKeyDown(int32_t key) const = 0;

    static IPlatform* GetInstance() {
        return instance_;
    }

private:
    static inline IPlatform* instance_ = nullptr;
};


using TimeSeconds = std::chrono::duration<float, std::ratio<1, 1>>;

inline std::chrono::milliseconds GetNow() {
    using namespace std::chrono;
    auto duration = steady_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(duration);
}

glm::vec2 GetMousePosition();
glm::vec2 GetLastMousePosition();

void SetMouseVisible(bool mouse_visible);
bool IsKeyDown(int32_t key);

using time_milliseconds_t = std::chrono::milliseconds;

class Layer {
public:
    virtual ~Layer() = default;

public:
    virtual void OnUpdate(time_milliseconds_t delta_time) = 0;
    virtual void OnRender(time_milliseconds_t delta_time) = 0;
    virtual bool OnEvent(const Event& event) = 0;
    virtual void OnImguiFrame() = 0;

    virtual std::type_index GetTypeIndex() const {
        return typeid(Layer);
    }
};
