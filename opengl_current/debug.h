#pragma once

#include "transform.h"
#include "box.h"
#include "submit_args.h"

class Debug
{
public:
    static void Quit();
    static void BeginScene(const glm::mat4& projectionViewMatrix);

    static void DrawDebugBox(const Box& box, const Transform& transform, const glm::vec4& color = glm::vec4{1, 1, 1, 1});
    static void DrawDebugLine(const Line& line, const Transform& transform, const glm::vec4& color = glm::vec4{1, 1, 1, 1});
    static void FlushDrawDebug();

    static void InitializeDebugDraw(const std::shared_ptr<Shader>& debugShader);
    
    static glm::mat4 GetProjectionViewMatrix()
    {
        return s_ProjectionViewMatrix;
    }

private:
    static glm::mat4 s_ProjectionViewMatrix;
};

