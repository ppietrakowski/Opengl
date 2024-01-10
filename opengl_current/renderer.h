#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"

#include <cstdint>

struct CameraProjection
{
    float Fov{45.0f};
    float AspectRatio{0.0f};
    float Width;
    float Height;
    float ZNear{0.1f};
    float ZFar{1000.0f};

    CameraProjection(std::int32_t Width, std::int32_t height, float fov = 45.0f, float z_near = 0.1f, float z_far = 1000.0f) :
        Width((float)Width),
        Height((float)height),
        Fov(fov),
        ZNear(z_near),
        ZFar(z_far)
    {
        AspectRatio = this->Width / this->Height;
    }

    CameraProjection() = default;
    CameraProjection(const CameraProjection&) = default;
    CameraProjection& operator=(const CameraProjection&) = default;
};

class Renderer
{
public:
    static void Initialize();
    static void Quit();

    static void UpdateProjection(const CameraProjection& projection);

public:
    static void BeginScene(const glm::mat4& view, glm::vec3 cameraPos, glm::quat cameraRotation);
    static void EndScene();

    static void Submit(const Material& material,
        const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f},
        RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);

    static void Submit(const Material& material,
        std::int32_t numIndices,
        const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f},
        RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);


    static void SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, std::int32_t numIndices,
        const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f},
        RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);

    static void Submit(Shader& shader,
        const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f},
        RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);

    static void Submit(Shader& shader,
        std::int32_t numIndices, const VertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f}, RenderPrimitive renderPrimitive = RenderPrimitive::Triangles);

    static std::shared_ptr<Texture2D> GetDefaultTexture();

    static void DrawDebugBox(glm::vec3 boxMin, glm::vec3 boxMax, const Transform& transform);
    static void DrawDebugBox(glm::vec3 boxMin, glm::vec3 boxMax, const Transform& transform, const glm::vec4& color);
    static void FlushDrawDebug(Material& shader);

    static bool IsVisibleToCamera(glm::vec3 worldspacePosition, glm::vec3 bboxMin, glm::vec3 bboxMax);

private:
    static glm::mat4 s_View;
    static glm::mat4 s_Projection;
    static glm::mat4 s_ViewProjection;
    static glm::vec3 s_CameraPosition;
    static std::shared_ptr<Texture2D> s_DefaultTexture;

    static void UploadUniforms(Shader& shader, const glm::mat4& transform);
};

inline std::shared_ptr<Texture2D> Renderer::GetDefaultTexture()
{
    return s_DefaultTexture;
}