#pragma once

#include "Keys.h"
#include <glm/glm.hpp>

class Input {
public:
	Input();
	virtual ~Input() = default;

public:

	static bool IsKeyPressed(KeyCode key);

	static bool IsMouseButtonPressed(MouseCode button);
	static glm::vec2 GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();

private:
	static Input* instance_;

	virtual bool IsKeyPressedImpl(KeyCode key) = 0;
	virtual bool IsMouseButtonPressedImpl(MouseCode button) = 0;
	virtual glm::vec2 GetMousePositionImpl() = 0;
};

