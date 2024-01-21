#pragma once

#include "event.h"
#include "input.h"
#include "duration.h"

#include <typeindex>
#include <chrono>


class Layer {
public:
    virtual ~Layer() = default;

public:
    virtual void Update(Duration delta_time) = 0;
    virtual void Render(Duration delta_time) = 0;
    virtual bool OnEvent(const Event& event) = 0;
    virtual void OnImguiFrame() = 0;
    virtual void OnImgizmoFrame() {
    }

    virtual std::type_index GetTypeIndex() const = 0;
};
