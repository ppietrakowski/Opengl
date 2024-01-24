#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"
#include "batching.h"
#include "camera_projection.h"

struct SpriteVertex
{
    glm::vec2 Position;
    glm::vec2 TextureCoords;
    int TextureId;
    RgbaColor Tint;
};

struct Transform2D
{
    glm::vec2 Position{0, 0};
    float Rotation{0};
    glm::vec2 Scale{1, 1};
    glm::vec2 Origin{0, 0};

    glm::mat4 GetTransformMatrix() const;
};

class Renderer2D
{
public:
    static void Initialize();
    static void Quit();

    static void SetDrawShader(const std::shared_ptr<Shader>& shader);

    static void UpdateProjection(const CameraProjection& projection);

    static void DrawRect(const glm::vec2& mins, const glm::vec2& maxs, const Transform2D& transform,
        const RgbaColor& Color, int textureId, glm::vec2 tilling = glm::vec2{1, 1});
    static void FlushDraw();

    static int BindTextureToDraw(const std::shared_ptr<Texture>& texture);
};