#pragma once

#include <functional>
#include <entt/entt.hpp>
#include "actor.h"
#include "event.h"

// Axis simulated by keys
struct KeyAxis
{
    KeyCode ForwardKey;
    KeyCode ReverseKey;

    float GetAxisValue() const;
};

class PlayerController
{
public:
    PlayerController(const Actor& actor);
    PlayerController(const PlayerController&) = default;
    PlayerController& operator=(const PlayerController&) = default;

    void BindFireCallback(const std::function<void(Actor& actor)>& fireCallback);
    void BindForwardCallback(const std::function<void(Actor& actor, float)>& forwardCallback);
    void BindRightCallback(const std::function<void(Actor& actor, float)>& rightCallback);

    void BindMouseMoveCallback(const std::function<void(Actor& actor, glm::vec2 delta)>& mouseMove);

    void Update();

    const Actor& GetActor() const
    {
        return m_PlayerActor;
    }

private:
    std::function<void(Actor& actor)> m_FireCallback;
    std::function<void(Actor& actor, float axisValue)> m_ForwardCallback;
    std::function<void(Actor& actor, float axisValue)> m_RightCallback;
    std::function<void(Actor& actor, glm::vec2 delta)> m_MouseMove;

    bool m_bFireButtonLastState{false};
    KeyAxis m_ForwardAxis{GLFW_KEY_W, GLFW_KEY_S};
    KeyAxis m_RightAxis{GLFW_KEY_D, GLFW_KEY_A};

    glm::vec2 m_LastMousePosition{0.0f, 0.0f};

    Actor m_PlayerActor;
};
