#include "player_controller.h"
#include "input.h"

float KeyAxis::GetAxisValue() const
{
    float value = Input::IsKeyPressed(ForwardKey) ? 1.0f : 0.0f;
    value -= Input::IsKeyPressed(ReverseKey) ? 1.0f : 0.0f;
    return value;
}

PlayerController::PlayerController(const Actor& actor) :
    m_PlayerActor{actor}
{
    m_LastMousePosition = Input::GetMousePosition();
}

void PlayerController::BindFireCallback(const std::function<void(Actor& actor)>& fireCallback)
{
    m_FireCallback = fireCallback;
}

void PlayerController::BindForwardCallback(const std::function<void(Actor& actor, float)>& forwardCallback)
{
    m_ForwardCallback = forwardCallback;
}

void PlayerController::BindRightCallback(const std::function<void(Actor& actor, float)>& rightCallback)
{
    m_RightCallback = rightCallback;
}

void PlayerController::BindMouseMoveCallback(const std::function<void(Actor& actor, glm::vec2 delta)>& mouseMove)
{
    m_MouseMove = mouseMove;
}

void PlayerController::Update()
{
    bool bNewFireButtonState = Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);

    if (bNewFireButtonState != m_bFireButtonLastState && !m_bFireButtonLastState)
    {
        if (m_FireCallback)
        {
            m_FireCallback(m_PlayerActor);
        }

        m_bFireButtonLastState = bNewFireButtonState;
    }

    float forwardValue = m_ForwardAxis.GetAxisValue();

    if (forwardValue != 0)
    {
        m_ForwardCallback(m_PlayerActor, forwardValue);
    }

    float rightValue = m_RightAxis.GetAxisValue();

    if (rightValue != 0)
    {
        m_RightCallback(m_PlayerActor, rightValue);
    }

    glm::vec2 mousePosition = Input::GetMousePosition();

    if (m_MouseMove)
    {
        m_MouseMove(m_PlayerActor, mousePosition - m_LastMousePosition);
    }

    m_LastMousePosition = mousePosition;
}
