#pragma once

#include "Transform.hpp"
#include "Box.hpp"
#include "Shader.hpp"

class Debug
{
    friend class Game;

private:
    static void Quit();

public:
    static void BeginScene(const glm::mat4& projectionViewMatrix);

    static void DrawDebugBox(const Box& box, const Transform& transform, const glm::vec4& color = glm::vec4{1, 1, 1, 1});
    static void DrawDebugLine(const Line& line, const Transform& transform, const glm::vec4& color = glm::vec4{1, 1, 1, 1});
    static void DrawDebugRect(glm::vec2 position, glm::vec2 size, const Transform& transform, const glm::vec4& color = glm::vec4{1, 1, 1, 1});
    static void FlushDrawDebug();

    static glm::mat4 GetProjectionViewMatrix()
    {
        return s_ProjectionViewMatrix;
    }

    static void InitializeDebugDraw(const std::shared_ptr<Shader>& debugShader);

private:
    static glm::mat4 s_ProjectionViewMatrix;
};

