#pragma once

#include "Material.hpp"
#include "VertexArray.hpp"
#include "Renderer2D.hpp"

constexpr int NumQuadVertices = 4;
constexpr int MaxSpritesDisplayed = 400;

struct SpriteVertex
{
    glm::vec2 Position;
    glm::vec2 TextureCoords;
    int TextureId;
    RgbaColor Tint;
};

class SpriteBatch
{
public:
    SpriteBatch(std::shared_ptr<Material> material, const glm::mat4& projectionCopy);

    void FlushDraw(const glm::mat4& projection);

    void AddSpriteInstance(const std::array<SpriteVertex, NumQuadVertices>& definition, const Transform2D& transform);
    void BindNewTexture(std::shared_ptr<ITexture> texture);

    int GetNumBindedTextures() const;

private:
    VertexArray m_SpriteVertexArray;
    std::vector<SpriteVertex> m_Sprites;

    std::array<std::shared_ptr<ITexture>, MinTextureUnits> m_BindTextures;
    int m_NumBindedTextures = 0;

    int m_LastIndex = 0;
    int m_NumIndicesToDraw = 0;
    std::shared_ptr<Material> m_Material2d;
    glm::mat4 m_ProjectionCopy;

private:

    void BindSpriteUniforms(const glm::mat4& projection);
    void Reset();
};


