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

struct SpriteBatch
{
    VertexArray SpriteVertexArray;
    std::vector<SpriteVertex> Sprites;

    std::array<std::shared_ptr<Texture>, MinTextureUnits> bind_textures;
    int NumBindedTextures = 0;

    int LastIndex = 0;
    int NumIndicesToDraw = 0;
    std::shared_ptr<Material> Material2d = nullptr;

    SpriteBatch(std::shared_ptr<Material> material) :
        Material2d(material)
    {
        material->bCullFaces = false;
        Sprites.reserve(MAX_SPRITES_DISPLAYED * NUM_QUAD_VERTICES);
        std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(static_cast<int>(Sprites.capacity() * sizeof(SpriteVertex)));

        SpriteVertexArray.AddVertexBuffer(buffer, SpriteVertexAttributes);

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
        SpriteVertexArray.SetIndexBuffer(indexBuffer);
    }

    void FlushDraw(const glm::mat4& projection)
    {
        std::shared_ptr<Shader> shader = Material2d->GetShader();

        RenderCommand::SetDepthEnabled(false);
        Material2d->SetupRenderState();

        shader->Use();

        Material2d->SetShaderUniforms();

        for (int i = 0; i < NumBindedTextures; ++i)
        {
            bind_textures[i]->Bind(i);
        }

        shader->SetSamplersUniform("u_textures", std::span<const std::shared_ptr<Texture>>{bind_textures.begin(), (size_t)NumBindedTextures});

        shader->SetUniform("u_projection", projection);
        auto vertexBuffer = SpriteVertexArray.GetVertexBufferAt(0);

        SpriteVertexArray.Bind();
        vertexBuffer->UpdateVertices(Sprites.data(), static_cast<int>(sizeof(SpriteVertex) * Sprites.size()));

        RenderCommand::DrawIndexed(SpriteVertexArray, NumIndicesToDraw);

        LastIndex = 0;
        NumBindedTextures = 0;
        NumIndicesToDraw = 0;
        Sprites.clear();

        RenderCommand::SetDepthEnabled(true);
        SpriteVertexArray.Unbind();
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
        bind_textures[NumBindedTextures++] = texture;
    }
};

static SpriteBatch* s_SpriteBatch = nullptr;
static glm::mat4 s_Projection{1.0f};

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

void Renderer2D::DrawRect(const glm::vec2& mins, const glm::vec2& maxs, const Transform2D& transform, const RgbaColor& Color, int textureId, glm::vec2 tilling)
{

    std::array vertices = {
        SpriteVertex{mins, tilling * glm::vec2{0.0f, 0.0f}, textureId, Color},
        SpriteVertex{glm::vec2{maxs.x, mins.y}, tilling * glm::vec2{1.0f, 0.0f}, textureId, Color},
        SpriteVertex{glm::vec2{maxs.x, maxs.y}, tilling * glm::vec2{1.0f, 1.0f}, textureId, Color},
        SpriteVertex{glm::vec2{mins.x, maxs.y}, tilling * glm::vec2{0.0f, 1.0f}, textureId, Color},
    };

    s_SpriteBatch->AddSpriteInstance(vertices, transform);
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
    const glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 rotationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{Origin, 0.0f});
    rotationMatrix = rotationMatrix * rotationZ;
    rotationMatrix = glm::translate(rotationMatrix, glm::vec3{-Origin, 0.0f});

    // TRS matrix
    return glm::translate(glm::mat4(1.0f), glm::vec3{Position + Origin, 0.0f}) *
        rotationMatrix * glm::scale(glm::mat4(1.0f), glm::vec3{Scale, 1});
}
