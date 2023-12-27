#pragma once

#include "event.h"
#include "input.h"
#include "duration.h"

#include <typeindex>
#include <chrono>

class IPlatform {
public:
    IPlatform() {
        instance_ = this;
    }
    virtual ~IPlatform() = default;

public:
    virtual void SetMouseVisible(bool mouse_visible) = 0;

    static IPlatform* GetInstance() {
        return instance_;
    }

private:
    static inline IPlatform* instance_ = nullptr;
};

glm::vec2 GetMousePosition();
glm::vec2 GetLastMousePosition();

void SetMouseVisible(bool mouse_visible);

class Layer {
public:
    virtual ~Layer() = default;

public:
    virtual void OnUpdate(Duration delta_time) = 0;
    virtual void OnRender(Duration delta_time) = 0;
    virtual bool OnEvent(const Event& event) = 0;
    virtual void OnImguiFrame() = 0;

    virtual std::type_index GetTypeIndex() const {
        return typeid(Layer);
    }
};
