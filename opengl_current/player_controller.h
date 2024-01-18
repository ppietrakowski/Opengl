#pragma once

#include <functional>
#include <entt/entt.hpp>
#include "keys.h"
#include "actor.h"

// Axis simulated by keys
struct KeyAxis
{
    KeyCode key;
    KeyCode reverse_key;

    float GetAxisValue() const;
};

class PlayerController
{
public:
    PlayerController(const Actor& actor);
    PlayerController(const PlayerController&) = default;
    PlayerController& operator=(const PlayerController&) = default;

    void BindFireCallback(const std::function<void(Actor& actor)>& fire_callback);
    void BindForwardCallback(const std::function<void(Actor& actor, float)>& forward_callback);
    void BindRightCallback(const std::function<void(Actor& actor, float)>& right_callback);

    void BindMouseMoveCallback(const std::function<void(Actor& actor, glm::vec2 delta)>& mouse_move);

    void Update();

    const Actor& GetActor() const
    {
        return player_actor_;
    }

private:
    std::function<void(Actor& actor)> fire_callback_;
    std::function<void(Actor& actor, float axisValue)> forward_callback_;
    std::function<void(Actor& actor, float axisValue)> right_callback_;
    std::function<void(Actor& actor, glm::vec2 delta)> mouse_move_;

    bool fire_button_last_state_{false};
    KeyAxis forward_axis_{Keys::kW, Keys::kS};
    KeyAxis right_axis_{Keys::kD, Keys::kA};

    glm::vec2 last_mouse_position_{0.0f, 0.0f};

    Actor player_actor_;
};
