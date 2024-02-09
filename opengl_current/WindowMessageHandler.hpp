#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "Keys.hpp"

class IWindowMessageHandler
{
public:
    virtual ~IWindowMessageHandler() = default;

public:
    virtual bool OnKeyDown(KeyCode::Index keyCode)
    {
        return false;
    }

    virtual bool OnKeyUp(KeyCode::Index keyCode)
    {
        return false;
    }

    virtual bool OnKeyRepeat(KeyCode::Index keyCode)
    {
        return false;
    }

    virtual bool OnMouseButtonDown(MouseButton mouseButton, glm::vec2 mousePosition)
    {
        return false;
    }

    virtual bool OnMouseButtonUp(MouseButton mouseButton, glm::vec2 mousePosition)
    {
        return false;
    }

    virtual bool OnMouseWheel(float delta)
    {
        return false;
    }

    virtual bool OnMouseMove(glm::vec2 mousePosition)
    {
        return false;
    }

    virtual bool OnSizeChanged(glm::uvec2 size)
    {
        return false;
    }

    virtual bool OnMovedWindow(glm::ivec2 newWindowPosition)
    {
        return false;
    }

    virtual bool OnWindowClose()
    {
        return false;
    }
};
