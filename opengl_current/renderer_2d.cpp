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
        m_Material2d(material)
    {
        m_SpriteVertexArray = std::make_shared<VertexArray>();

        material->bCullFaces = false;
        m_Sprites.reserve(MaxSpritesDisplayed * NumQuadVertices);
        std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(static_cast<int>(m_Sprites.capacity() * sizeof(SpriteVertex)));

        m_SpriteVertexArray->AddVertexBuffer(buffer, SpriteVertexAttributes);

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
        m_SpriteVertexArray->SetIndexBuffer(indexBuffer);
    }

    void FlushDraw(const glm::mat4& projection)
    {
        std::shared_ptr<Shader> shader = m_Material2d->GetShader();

        // just prevents UI from being culled
        m_Material2d->SetupRenderState();
        DepthTestDisabler depthTestDisabler{};

        BindSpriteUniforms(projection);

        std::shared_ptr<VertexBuffer> vertexBuffer = m_SpriteVertexArray->GetVertexBufferAt(0);

        m_SpriteVertexArray->Bind();
        vertexBuffer->UpdateVertices(m_Sprites.data(), static_cast<int>(sizeof(SpriteVertex) * m_Sprites.size()));

        RenderCommand::DrawIndexed(m_SpriteVertexArray, m_NumIndicesToDraw);
        Reset();
    }

    void AddSpriteInstance(const std::array<SpriteVertex, NumQuadVertices>& definition, const Transform2D& transform)
    {
        glm::mat4 transformMatrix = transform.GetTransformMatrix();

        for (const SpriteVertex& sprite_vertex : definition)
        {
            SpriteVertex vertex = sprite_vertex;
            vertex.Position = transformMatrix * glm::vec4(vertex.Position, 0.5f, 1);
            m_Sprites.emplace_back(vertex);
        }

        m_NumIndicesToDraw += 6;
    }

    void BindNewTexture(std::shared_ptr<Texture> texture)
    {
        if (m_NumBindedTextures >= MinTextureUnits)
        {
            FlushDraw(s_Projection);
            m_NumBindedTextures = 0;
        }

        m_BindTextures[m_NumBindedTextures++] = texture;
    }

    int GetNumBindedTextures() const
    {
        return m_NumBindedTextures;
    }

private:
    std::shared_ptr<VertexArray> m_SpriteVertexArray;
    std::vector<SpriteVertex> m_Sprites;

    std::array<std::shared_ptr<Texture>, MinTextureUnits> m_BindTextures;
    int m_NumBindedTextures = 0;

    int m_LastIndex = 0;
    int m_NumIndicesToDraw = 0;
    std::shared_ptr<Material> m_Material2d;

private:

    void BindSpriteUniforms(const glm::mat4& projection)
    {
        std::shared_ptr<Shader> shader = m_Material2d->GetShader();
        shader->Use();
        m_Material2d->SetShaderUniforms();

        // bind all attached textures
        for (int i = 0; i < m_NumBindedTextures; ++i)
        {
            m_BindTextures[i]->Bind(i);
        }

        shader->SetSamplersUniform("u_Textures", std::span<const std::shared_ptr<Texture>>{m_BindTextures.begin(), (size_t)m_NumBindedTextures});
        shader->SetUniform("u_Projection", projection);
    }

    void Reset()
    {
        m_LastIndex = 0;
        m_NumBindedTextures = 0;
        m_NumIndicesToDraw = 0;
        m_Sprites.clear();
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