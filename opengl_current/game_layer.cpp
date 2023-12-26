#include "game_layer.h"
#include "error_macros.h"

void SetMouseVisible(bool mouse_visible) {
    ERR_FAIL_EXPECTED_TRUE(IPlatform::GetInstance() != nullptr);
    IPlatform::GetInstance()->SetMouseVisible(mouse_visible);
}