#include "renderer_2d.h"
#include "error_macros.h"

#include <glm/gtc/matrix_transform.hpp>

static const std::array<VertexAttribute, 4> SpriteVertexAttributes{
    VertexAttribute{2, PrimitiveVertexType::Float},
    VertexAttribute{2, PrimitiveVertexType::Float},
    VertexAttribute{1, PrimitiveVertexType::Int},
    VertexAttribute{1, PrimitiveVertexType::UnsignedInt}
};

constexpr size_t MaxSpritesDisplayed = 400;

static glm::mat4 s_Projection{1.0f};

struct DepthTestDisabler
{
    DepthTestDisabler()
    {
        RenderCommand::SetDepthEnabled(false);
    }

    ~DepthTestDisabler()
    {
        RenderCommand::SetDepthEnabled(true);
    }
};

class SpriteBatch
{
public:
    SpriteBatch(std::shared_ptr<Material> material) :
        Material2d(material)
    {
        SpriteVertexArray = std::make_shared<VertexArray>();

        material->bCullFaces = false;
        Sprites.reserve(MaxSpritesDisplayed * NumQuadVertices);
        std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(static_cast<int>(Sprites.capacity() * sizeof(SpriteVertex)));

        SpriteVertexArray->AddVertexBuffer(buffer, SpriteVertexAttributes);

        uint32_t startIndex = 0;
        constexpr std::array<uint32_t, 6> BaseQuatIndices = {0, 1, 2, 0, 2, 3};

        std::vector<uint32_t> batchedIndices;

        batchedIndices.reserve(BaseQuatIndices.size() * MaxSpritesDisplayed);

        for (uint32_t i = 0; i < MaxSpritesDisplayed; ++i)
        {
            for (uint32_t index : BaseQuatIndices)
            {
                batchedIndices.emplace_back(index + startIndex);
            }

            startIndex += NumQuadVertices;
        }

        std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(batchedIndices.data(), STD_ARRAY_NUM_ELEMENTS(batchedIndices));
        SpriteVertexArray->SetIndexBuffer(indexBuffer);
    }

    void FlushDraw(const glm::mat4& projection)
    {
        std::shared_ptr<Shader> shader = Material2d->GetShader();

        // just prevents UI from being culled
        Material2d->SetupRenderState();
        DepthTestDisabler depthTestDisabler{};

        BindSpriteUniforms(projection);

        std::shared_ptr<VertexBuffer> vertexBuffer = SpriteVertexArray->GetVertexBufferAt(0);

        SpriteVertexArray->Bind();
        vertexBuffer->UpdateVertices(Sprites.data(), static_cast<int>(sizeof(SpriteVertex) * Sprites.size()));

        RenderCommand::DrawIndexed(SpriteVertexArray, NumIndicesToDraw);
        Reset();
    }

    void AddSpriteInstance(const std::array<SpriteVertex, NumQuadVertices>& definition, const Transform2D& transform)
    {
        glm::mat4 transformMatrix = transform.GetTransformMatrix();

        for (const SpriteVertex& sprite_vertex : definition)
        {
            SpriteVertex vertex = sprite_vertex;
            vertex.Position = transformMatrix * glm::vec4(vertex.Position, 0.5f, 1);
            Sprites.emplace_back(vertex);
        }

        NumIndicesToDraw += 6;
    }

    void BindNewTexture(std::shared_ptr<Texture> texture)
    {
        if (NumBindedTextures >= MinTextureUnits)
        {
            FlushDraw(s_Projection);
            NumBindedTextures = 0;
        }

        BindTextures[NumBindedTextures++] = texture;
    }

    int GetNumBindedTextures() const
    {
        return NumBindedTextures;
    }

private:
    std::shared_ptr<VertexArray> SpriteVertexArray;
    std::vector<SpriteVertex> Sprites;

    std::array<std::shared_ptr<Texture>, MinTextureUnits> BindTextures;
    int NumBindedTextures = 0;

    int LastIndex = 0;
    int NumIndicesToDraw = 0;
    std::shared_ptr<Material> Material2d;

private:

    void BindSpriteUniforms(const glm::mat4& projection)
    {
        std::shared_ptr<Shader> shader = Material2d->GetShader();
        shader->Use();
        Material2d->SetShaderUniforms();

        // bind all attached textures
        for (int i = 0; i < NumBindedTextures; ++i)
        {
            BindTextures[i]->Bind(i);
        }

        shader->SetSamplersUniform("u_textures", std::span<const std::shared_ptr<Texture>>{BindTextures.begin(), (size_t)NumBindedTextures});
        shader->SetUniform("u_projection", projection);
    }

    void Reset()
    {
        LastIndex = 0;
        NumBindedTextures = 0;
        NumIndicesToDraw = 0;
        Sprites.clear();
    }
};

SpriteSheetData::SpriteSheetData(glm::uvec2 numFrames, glm::vec2 margin, glm::vec2 spriteSheetSize, const std::shared_ptr<Texture2D>& texture) :
    m_NumFrames(numFrames),
    m_Margin(margin),
    m_SpriteSheetSize(spriteSheetSize),
    m_Texture(texture)
{
    ASSERT(m_SpriteSheetSize.x <= m_Texture->GetWidth() && m_SpriteSheetSize.y <= m_Texture->GetHeight());
}

Sprite2D::Sprite2D(glm::vec2 position, glm::vec2 size, int textureId, const SpriteSheetData& spriteSheetData, glm::uvec2 animationFrame, RgbaColor tint) :
    Transform{position, 0.0f, size, size / 2.0f},
    Tint(tint),
    TextureId(textureId),
    AnimationFrame(animationFrame),
    SpriteSheetInfo(spriteSheetData)
{
}

static SpriteBatch* s_SpriteBatch = nullptr;

static constexpr glm::vec2 SpriteVertexPositions[4] =
{{  0.0f, 0.0f },
 {  1.0f, 0.0f },
 {  1.0f,  1.0f },
 {  0.0f,  1.0f }};

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
    s_SpriteBatch = new SpriteBatch(std::make_shared<Material>(shader));
}

void Renderer2D::UpdateProjection(const CameraProjection& projection)
{
    s_Projection = glm::ortho(0.0f, projection.Width, 0.0f, projection.Height, -1.0f, 1.0f);
}

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

int Renderer2D::BindTextureToDraw(const std::shared_ptr<Texture>& texture)
{
    s_SpriteBatch->BindNewTexture(texture);
    return s_SpriteBatch->GetNumBindedTextures() - 1;
}

glm::mat4 Transform2D::GetTransformMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(Position, 0.0f));

    model = glm::translate(model, glm::vec3(Origin, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-Origin, 0.0f));

    model = glm::scale(model, glm::vec3(Size, 1.0f));

    return model;
}