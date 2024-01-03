#pragma once

#include "event.h"
#include "input.h"
#include "duration.h"

#include <typeindex>
#include <chrono>


class ILayer
{
public:
    virtual ~ILayer() = default;

public:
    virtual void Update(Duration deltaTime) = 0;
    virtual void Render(Duration deltaTime) = 0;
    virtual bool OnEvent(const Event& event) = 0;
    virtual void OnImguiFrame() = 0;

    virtual std::type_index GetTypeIndex() const
    {
        return typeid(ILayer);
    }
};
