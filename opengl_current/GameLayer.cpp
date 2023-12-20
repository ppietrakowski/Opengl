#include "GameLayer.h"
#include "ErrorMacros.h"

glm::vec2 GetMousePosition() {
    ERR_FAIL_EXPECTED_TRUE_V(IPlatform::GetInstance() != nullptr, glm::vec2(0, 0));
    return IPlatform::GetInstance()->GetMousePosition();
}

glm::vec2 GetLastMousePosition() {
    ERR_FAIL_EXPECTED_TRUE_V(IPlatform::GetInstance() != nullptr, glm::vec2(0, 0));
    return IPlatform::GetInstance()->GetLastMousePosition();
}

void SetMouseVisible(bool mouse_visible) {
    ERR_FAIL_EXPECTED_TRUE(IPlatform::GetInstance() != nullptr);
    IPlatform::GetInstance()->SetMouseVisible(mouse_visible);
}

bool IsKeyDown(std::int32_t key) {
    ERR_FAIL_EXPECTED_TRUE_V(IPlatform::GetInstance() != nullptr, false);
    return IPlatform::GetInstance()->IsKeyDown(key);
}
