#include "Renderer2D.hpp"
#include "ErrorMacros.hpp"
#include "SpriteBatch.hpp"

#include <glm/gtc/matrix_transform.hpp>

static glm::mat4 s_Projection{1.0f};

static SpriteBatch* s_SpriteBatch = nullptr;

void Renderer2D::Initialize()
{
}

void Renderer2D::Quit()
{
    SafeDelete(s_SpriteBatch);
}

void Renderer2D::SetDrawShader(const std::shared_ptr<Shader>& shader)
{
    SafeDelete(s_SpriteBatch);
    s_SpriteBatch = new SpriteBatch(std::make_shared<Material>(shader), s_Projection);
}

void Renderer2D::UpdateProjection(const CameraProjection& projection)
{
    s_Projection = glm::ortho(0.0f, projection.Width, 0.0f, projection.Height, -1.0f, 1.0f);
}

static constexpr glm::vec2 SpriteVertexPositions[4] =
{{  0.0f, 0.0f },
 {  1.0f, 0.0f },
 {  1.0f,  1.0f },
 {  0.0f,  1.0f }};

void Renderer2D::DrawSprite(const Sprite2D& definition)
{
    ASSERT(s_SpriteBatch);
    ASSERT(definition.TextureId < s_SpriteBatch->GetNumBindedTextures());

    glm::vec2 start = definition.SpriteSheetInfo.GetStartUvCoordinate(definition.AnimationFrame);
    glm::vec2 end = definition.SpriteSheetInfo.GetEndUvCoordinate(definition.AnimationFrame);

    std::array<SpriteVertex, NumQuadVertices> vertices = {
        SpriteVertex{SpriteVertexPositions[0], start, definition.TextureId, definition.Tint},
        SpriteVertex{SpriteVertexPositions[1], glm::vec2(end.x, start.y), definition.TextureId, definition.Tint},
        SpriteVertex{SpriteVertexPositions[2], end, definition.TextureId, definition.Tint},
        SpriteVertex{SpriteVertexPositions[3], glm::vec2(start.x, end.y), definition.TextureId, definition.Tint}
    };

    s_SpriteBatch->AddSpriteInstance(vertices, definition.Transform);
}

void Renderer2D::FlushDraw()
{
    s_SpriteBatch->FlushDraw(s_Projection);
}

std::int32_t Renderer2D::BindTextureToDraw(const std::shared_ptr<ITexture>& texture)
{
    s_SpriteBatch->BindNewTexture(texture);
    return s_SpriteBatch->GetNumBindedTextures() - 1;
}