#include "renderer_2d.h"

#include <glm/gtc/matrix_transform.hpp>

static const std::array SpriteVertexAttributes{
    VertexAttribute{2, PrimitiveVertexType::Float},
    VertexAttribute{2, PrimitiveVertexType::Float},
    VertexAttribute{1, PrimitiveVertexType::Int},
    VertexAttribute{1, PrimitiveVertexType::UnsignedInt}
};

#define MAX_SPRITES_DISPLAYED 400

#define NUM_QUAD_VERTICES 4

static glm::mat4 s_Projection{1.0f};

struct SpriteBatch
{
    std::shared_ptr<VertexArray> SpriteVertexArray;
    std::vector<SpriteVertex> Sprites;

    std::array<std::shared_ptr<Texture>, MinTextureUnits> BindTextures;
    int NumBindedTextures = 0;

    int LastIndex = 0;
    int NumIndicesToDraw = 0;
    std::shared_ptr<Material> Material2d = nullptr;

    SpriteBatch(std::shared_ptr<Material> material) :
        Material2d(material)
    {
        SpriteVertexArray = std::make_shared<VertexArray>();

        material->bCullFaces = false;
        Sprites.reserve(MAX_SPRITES_DISPLAYED * NUM_QUAD_VERTICES);
        std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(static_cast<int>(Sprites.capacity() * sizeof(SpriteVertex)));

        SpriteVertexArray->AddVertexBuffer(buffer, SpriteVertexAttributes);

        uint32_t startIndex = 0;
        uint32_t indices[] = {0, 1, 2, 0, 2, 3};

        std::vector<uint32_t> batchedIndices;

        batchedIndices.reserve(ARRAY_NUM_ELEMENTS(indices) * MAX_SPRITES_DISPLAYED);

        for (uint32_t i = 0; i < MAX_SPRITES_DISPLAYED; ++i)
        {
            for (uint32_t index : indices)
            {
                batchedIndices.emplace_back(index + startIndex);
            }

            startIndex += NUM_QUAD_VERTICES;
        }

        std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(batchedIndices.data(), (int)batchedIndices.size());
        SpriteVertexArray->SetIndexBuffer(indexBuffer);
    }

    void FlushDraw(const glm::mat4& projection)
    {
        std::shared_ptr<Shader> shader = Material2d->GetShader();

        Material2d->SetupRenderState();

        shader->Use();

        Material2d->SetShaderUniforms();

        for (int i = 0; i < NumBindedTextures; ++i)
        {
            BindTextures[i]->Bind(i);
        }

        shader->SetSamplersUniform("u_textures", std::span<const std::shared_ptr<Texture>>{BindTextures.begin(), (size_t)NumBindedTextures});

        shader->SetUniform("u_projection", projection);
        auto vertexBuffer = SpriteVertexArray->GetVertexBufferAt(0);

        SpriteVertexArray->Bind();
        vertexBuffer->UpdateVertices(Sprites.data(), static_cast<int>(sizeof(SpriteVertex) * Sprites.size()));

        RenderCommand::DrawIndexed(SpriteVertexArray, NumIndicesToDraw);

        LastIndex = 0;
        NumBindedTextures = 0;
        NumIndicesToDraw = 0;
        Sprites.clear();
    }

    void AddSpriteInstance(const std::array<SpriteVertex, NUM_QUAD_VERTICES>& definition, const Transform2D& transform)
    {
        glm::mat4 transformMatrix = transform.GetTransformMatrix();

        for (const SpriteVertex& sprite_vertex : definition)
        {
            SpriteVertex vertex = sprite_vertex;
            vertex.Position = transformMatrix * glm::vec4(vertex.Position, 0, 1);
            Sprites.emplace_back(vertex);
        }

        NumIndicesToDraw += 6;
    }

    void BindNewTexture(std::shared_ptr<Texture> texture)
    {
        if (NumBindedTextures == MinTextureUnits)
        {
            FlushDraw(s_Projection);
        }

        BindTextures[NumBindedTextures++] = texture;
    }
};

SpriteSheetData::SpriteSheetData(glm::uvec2 numFrames, glm::vec2 margin, glm::vec2 spriteSheetSize, const std::shared_ptr<Texture2D>& texture) :
    m_NumFrames(numFrames),
    m_Margin(margin),
    m_SpriteSheetSize(spriteSheetSize),
    m_Texture(texture)
{
    assert(m_SpriteSheetSize.x <= m_Texture->GetWidth() && m_SpriteSheetSize.y <= m_Texture->GetHeight());
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

void Renderer2D::Initialize()
{
}

void Renderer2D::Quit()
{
    SafeDelete(s_SpriteBatch);
}

void Renderer2D::SetDrawShader(const std::shared_ptr<Shader>& shader)
{
    s_SpriteBatch = new SpriteBatch(std::make_shared<Material>(shader));
}

void Renderer2D::UpdateProjection(const CameraProjection& projection)
{
    s_Projection = glm::ortho(0.0f, projection.Width, 0.0f, projection.Height, -1.0f, 1.0f);
}

static constexpr glm::vec2 SpriteVertexPositions[] = 
        {{  0.0f, 0.0f },
         {  1.0f, 0.0f },
         {  1.0f,  1.0f },
         {  0.0f,  1.0f }};


void Renderer2D::DrawSprite(const Sprite2D& definition)
{
    assert(definition.TextureId < s_SpriteBatch->NumBindedTextures);

    glm::vec2 start = definition.SpriteSheetInfo.GetStartUvCoordinate(definition.AnimationFrame);
    glm::vec2 end = definition.SpriteSheetInfo.GetEndUvCoordinate(definition.AnimationFrame);

    std::array<SpriteVertex, 4> vertices = {
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
    return s_SpriteBatch->NumBindedTextures - 1;
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