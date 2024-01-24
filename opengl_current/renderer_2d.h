#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"
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
    glm::vec2 Size{1, 1};
    glm::vec2 Origin{0, 0};

    glm::mat4 GetTransformMatrix() const;
};

constexpr size_t NumQuadVertices = 4;

class SpriteSheetData
{
public:
    SpriteSheetData() = default;
    SpriteSheetData(glm::uvec2 numFrames, glm::vec2 margin, glm::vec2 spriteSheetSize, const std::shared_ptr<Texture2D>& texture);
    SpriteSheetData(const SpriteSheetData& data) = default;
    SpriteSheetData& operator=(const SpriteSheetData& data) = default;

    glm::vec2 GetStartUvCoordinate(glm::uvec2 frameCoords) const
    {
        glm::vec2 start((float)frameCoords.x / m_NumFrames.x, (float)frameCoords.y / m_NumFrames.y);

        start.x = start.x + m_Margin.x / m_SpriteSheetSize.x;
        start.y = start.y + m_Margin.y / m_SpriteSheetSize.y;

        return start;
    }

    glm::vec2 GetEndUvCoordinate(glm::uvec2 frameCoords) const
    {
        glm::vec2 end((float)(frameCoords.x + 1) / m_NumFrames.x, (float)(frameCoords.y + 1) / m_NumFrames.y);

        end.x = end.x - m_Margin.x / m_SpriteSheetSize.x;
        end.y = end.y - m_Margin.y / m_SpriteSheetSize.y;

        return end;
    }

private:
    // Max num frames in texture (MaxNumColumns, MaxNumRows)
    glm::uvec2 m_NumFrames{1, 1};
    // In pixels space
    glm::vec2 m_Margin = glm::vec2{0, 0};
    glm::vec2 m_SpriteSheetSize{1, 1};
    std::shared_ptr<Texture2D> m_Texture;
};

struct Sprite2D
{
    Transform2D Transform;
    RgbaColor Tint;
    int TextureId{0};

    // Current frame (Column, Row)
    glm::uvec2 AnimationFrame{0, 0};

    SpriteSheetData SpriteSheetInfo;

    Sprite2D() = default;
    Sprite2D(glm::vec2 position, glm::vec2 size, int textureId, const SpriteSheetData& spriteSheetData, glm::uvec2 animationFrame = {0, 0}, RgbaColor tint = RgbaColor{});
};

class Renderer2D
{
public:
    static void Initialize();
    static void Quit();

    static void SetDrawShader(const std::shared_ptr<Shader>& shader);

    static void UpdateProjection(const CameraProjection& projection);

    static void DrawSprite(const Sprite2D& sprite);
    static void FlushDraw();

    static int BindTextureToDraw(const std::shared_ptr<Texture>& texture);
};