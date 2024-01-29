#include "SpriteBatch.h"

static const std::array<VertexAttribute, 4> SpriteVertexAttributes{
    VertexAttribute{2, PrimitiveVertexType::Float},
    VertexAttribute{2, PrimitiveVertexType::Float},
    VertexAttribute{1, PrimitiveVertexType::Int},
    VertexAttribute{1, PrimitiveVertexType::UnsignedInt}
};

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

SpriteBatch::SpriteBatch(std::shared_ptr<Material> material, const glm::mat4& projectionCopy) :
    m_Material2d(material),
    m_ProjectionCopy(projectionCopy)
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

void SpriteBatch::FlushDraw(const glm::mat4& projection)
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

    m_ProjectionCopy = projection;
}

void SpriteBatch::AddSpriteInstance(const std::array<SpriteVertex, NumQuadVertices>& definition, const Transform2D& transform)
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

void SpriteBatch::BindNewTexture(std::shared_ptr<Texture> texture)
{
    if (m_NumBindedTextures >= MinTextureUnits)
    {
        FlushDraw(m_ProjectionCopy);
        m_NumBindedTextures = 0;
    }

    m_BindTextures[m_NumBindedTextures++] = texture;
}

std::int32_t SpriteBatch::GetNumBindedTextures() const
{
    return m_NumBindedTextures;
}

void SpriteBatch::BindSpriteUniforms(const glm::mat4& projection)
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

void SpriteBatch::Reset()
{
    m_LastIndex = 0;
    m_NumBindedTextures = 0;
    m_NumIndicesToDraw = 0;
    m_Sprites.clear();
}
