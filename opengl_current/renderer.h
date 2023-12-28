#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"

#include <cstdint>

struct CameraProjection
{
    float Fov{45.0f};
    float AspectRatio{0.0f};
    float Width;
    float Height;
    float ZNear{0.1f};
    float ZFar{1000.0f};

    CameraProjection(int32_t width, int32_t height, float fov=45.0f, float zNear=0.1f, float zFar=1000.0f):
        Width((float)width),
        Height((float)height),
        Fov(fov),
        ZNear(zNear),
        ZFar(zFar)
    {
        AspectRatio = Width / Height;
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
    static void BeginScene(const glm::mat4& view, glm::vec3 cameraPosition);
    static void EndScene();

    static void Submit(const Material& material,
        const IVertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f},
        RenderPrimitive renderPrimitive = RenderPrimitive::kTriangles);

    static void SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, int32_t numIndices,
        const IVertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f},
        RenderPrimitive renderPrimitive = RenderPrimitive::kTriangles);


    static void Submit(IShader& shader,
        const IVertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f},
        RenderPrimitive renderPrimitive = RenderPrimitive::kTriangles);

    static void Submit(IShader& shader,
        int32_t numIndices, const IVertexArray& vertexArray,
        const glm::mat4& transform = glm::mat4{1.0f}, RenderPrimitive renderPrimitive = RenderPrimitive::kTriangles);

    static std::shared_ptr<ITexture2D> GetDefaultTexture();

    static void DrawDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform);
    static void FlushDrawDebug(IShader& shader);

private:
    static glm::mat4 s_View;
    static glm::mat4 s_Projection;
    static glm::mat4 s_ProjectionView;
    static glm::vec3 s_CameraPosition;
    static std::shared_ptr<ITexture2D> s_DefaultTexture;

    static void UploadUniforms(IShader& shader, const glm::mat4& transform);
};

inline std::shared_ptr<ITexture2D> Renderer::GetDefaultTexture()
{
    return s_DefaultTexture;
}