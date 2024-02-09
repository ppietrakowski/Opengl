#pragma once

#include "Texture.hpp"
#include "Transform2D.hpp"
#include "RenderCommand.hpp"

class SpriteSheetData
{
public:
    SpriteSheetData() = default;
    SpriteSheetData(glm::uvec2 numFrames, glm::vec2 margin, glm::vec2 spriteSheetSize, const std::shared_ptr<Texture2D>& texture);
    SpriteSheetData(const SpriteSheetData& data) = default;
    SpriteSheetData& operator=(const SpriteSheetData& data) = default;

    glm::vec2 GetStartUvCoordinate(glm::uvec2 frameCoords) const;
    glm::vec2 GetEndUvCoordinate(glm::uvec2 frameCoords) const;

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


