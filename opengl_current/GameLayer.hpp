#pragma once

#include "WindowMessageHandler.hpp"
#include "Input.hpp"
#include "Duration.hpp"

#include <typeindex>
#include <chrono>


class IGameLayer : public IWindowMessageHandler
{
public:
    virtual ~IGameLayer() = default;

public:
    virtual void Update(Duration deltaTime) = 0;
    virtual void Render() = 0;
    virtual void OnImguiFrame() = 0;
    virtual void OnImgizmoFrame()
    {
    }

    virtual std::type_index GetTypeIndex() const = 0;
};
