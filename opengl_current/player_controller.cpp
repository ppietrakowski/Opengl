#include "player_controller.h"
#include "input.h"

float KeyAxis::GetAxisValue() const
{
    float value = Input::IsKeyPressed(key) ? 1.0f : 0.0f;
    value -= Input::IsKeyPressed(reverse_key) ? 1.0f : 0.0f;
    return value;
}

PlayerController::PlayerController(const Actor& actor) :
    player_actor_{actor}
{
    last_mouse_position_ = Input::GetMousePosition();
}

void PlayerController::BindFireCallback(const std::function<void(Actor& actor)>& fire_callback)
{
    fire_callback_ = fire_callback;
}

void PlayerController::BindForwardCallback(const std::function<void(Actor& actor, float)>& forward_callback)
{
    forward_callback_ = forward_callback;
}

void PlayerController::BindRightCallback(const std::function<void(Actor& actor, float)>& right_callback)
{
    right_callback_ = right_callback;
}

void PlayerController::BindMouseMoveCallback(const std::function<void(Actor& actor, glm::vec2 delta)>& mouse_move)
{
    mouse_move_ = mouse_move;
}

void PlayerController::Update()
{
    bool new_fire_button_state = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);

    if (new_fire_button_state != fire_button_last_state_ && !fire_button_last_state_)
    {
        if (fire_callback_)
        {
            fire_callback_(player_actor_);
        }

        fire_button_last_state_ = new_fire_button_state;
    }
    
    float forward_value = forward_axis_.GetAxisValue();

    if (forward_value != 0)
    {
        forward_callback_(player_actor_, forward_value);
    }

    float right_value = right_axis_.GetAxisValue();

    if (right_value != 0)
    {
        right_callback_(player_actor_, right_value);
    }

    glm::vec2 mouse_position = Input::GetMousePosition();

    if (mouse_move_)
    {
        mouse_move_(player_actor_, mouse_position - last_mouse_position_);
    }

    last_mouse_position_ = mouse_position;
}
