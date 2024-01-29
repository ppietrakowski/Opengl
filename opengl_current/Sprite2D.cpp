#include "Sprite2D.h"
#include "error_macros.h"

SpriteSheetData::SpriteSheetData(glm::uvec2 numFrames, glm::vec2 margin, glm::vec2 spriteSheetSize, const std::shared_ptr<Texture2D>& texture) :
    m_NumFrames(numFrames),
    m_Margin(margin),
    m_SpriteSheetSize(spriteSheetSize),
    m_Texture(texture)
{
    ASSERT(m_SpriteSheetSize.x <= m_Texture->GetWidth() && m_SpriteSheetSize.y <= m_Texture->GetHeight());
}

glm::vec2 SpriteSheetData::GetStartUvCoordinate(glm::uvec2 frameCoords) const
{
    glm::vec2 start((float)frameCoords.x / m_NumFrames.x, (float)frameCoords.y / m_NumFrames.y);

    start.x = start.x + m_Margin.x / m_SpriteSheetSize.x;
    start.y = start.y + m_Margin.y / m_SpriteSheetSize.y;

    return start;
}

glm::vec2 SpriteSheetData::GetEndUvCoordinate(glm::uvec2 frameCoords) const
{
    glm::vec2 end((float)(frameCoords.x + 1) / m_NumFrames.x, (float)(frameCoords.y + 1) / m_NumFrames.y);

    end.x = end.x - m_Margin.x / m_SpriteSheetSize.x;
    end.y = end.y - m_Margin.y / m_SpriteSheetSize.y;

    return end;
}

Sprite2D::Sprite2D(glm::vec2 position, glm::vec2 size, int textureId, const SpriteSheetData& spriteSheetData, glm::uvec2 animationFrame, RgbaColor tint) :
    Transform{position, 0.0f, size, size / 2.0f},
    Tint(tint),
    TextureId(textureId),
    AnimationFrame(animationFrame),
    SpriteSheetInfo(spriteSheetData)
{
}