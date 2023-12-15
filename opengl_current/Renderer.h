#pragma once

#include "RenderCommand.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Material.h"

#include <cstdint>

class Renderer
{
public:
    static void Initialize();
    static void Quit();

    static void UpdateProjection(float width, float height, float fov, float zNear = 0.1f, float zFar = 1000.0f);

public:
    static void BeginScene(const glm::mat4& view, glm::vec3 cameraPosition);
    static void EndScene();

    static void Submit(const Material& material,
        const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{ 1.0f },
        RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);

    static void Submit(Shader& shader,
        const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{ 1.0f },
        RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);

    static void Submit(Shader& shader,
        std::uint32_t numIndices, const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{ 1.0f }, RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);

    static std::shared_ptr<Texture2D> GetDefaultTexture();

    static void AddDebugBox(glm::vec3 min, glm::vec3 max, const glm::mat4& transform);
    static void FlushDrawDebug(Shader& shader);

private:
    static glm::mat4 View;
    static glm::mat4 Projection;
    static glm::mat4 ProjectionView;
    static glm::vec3 CameraLocation;
    static std::shared_ptr<Texture2D> DefaultTexture;
};

inline std::shared_ptr<Texture2D> Renderer::GetDefaultTexture()
{
    return DefaultTexture;
}