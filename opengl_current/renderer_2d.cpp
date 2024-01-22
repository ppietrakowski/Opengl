#include "renderer_2d.h"

#include <glm/gtc/matrix_transform.hpp>

static const std::array kSpriteVertexAttributes{
    VertexAttribute{2, PrimitiveVertexType::kFloat},
    VertexAttribute{2, PrimitiveVertexType::kFloat},
    VertexAttribute{1, PrimitiveVertexType::kInt},
    VertexAttribute{1, PrimitiveVertexType::kUnsignedInt}
};

#define MAX_SPRITES_DISPLAYED 400

#define NUM_QUAD_VERTICES 4

struct SpriteBatch {
    VertexArray vertex_array;

    std::vector<SpriteVertex> sprites;

    std::array<std::shared_ptr<Texture>, kMinTextureUnits> bind_textures;
    int num_binded_textures = 0;

    int last_index = 0;
    int num_indices_to_draw = 0;
    std::shared_ptr<Material> material_2d = nullptr;

    SpriteBatch(std::shared_ptr<Material> material) :
        material_2d(material) {
        material->cull_faces = false;
        sprites.reserve(MAX_SPRITES_DISPLAYED * NUM_QUAD_VERTICES);
        std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(sprites.capacity() * sizeof(SpriteVertex));

        vertex_array.AddVertexBuffer(buffer, kSpriteVertexAttributes);

        uint32_t start_index = 0;
        uint32_t indices[] = {0, 1, 2, 0, 2, 3};

        std::vector<uint32_t> batched_indices;

        batched_indices.reserve(ARRAY_NUM_ELEMENTS(indices) * MAX_SPRITES_DISPLAYED);

        for (uint32_t i = 0; i < MAX_SPRITES_DISPLAYED; ++i) {
            for (uint32_t index : indices) {
                batched_indices.emplace_back(index + start_index);
            }

            start_index += NUM_QUAD_VERTICES;
        }

        std::shared_ptr<IndexBuffer> index_buffer = std::make_shared<IndexBuffer>(batched_indices.data(), (int)batched_indices.size());
        vertex_array.SetIndexBuffer(index_buffer);
    }

    ~SpriteBatch() {
    }

    void FlushDraw(const glm::mat4 &projection) {
        Shader& shader = material_2d->GetShader();

        shader.Use();

        material_2d->SetupRenderState();
        material_2d->SetShaderUniforms();

        for (int i = 0; i < num_binded_textures; ++i) {
            bind_textures[i]->Bind(i);
        }

        shader.SetSamplersUniform("u_textures", std::span<const std::shared_ptr<Texture>>{bind_textures.begin(), (size_t)num_binded_textures});

        shader.SetUniform("u_projection", projection);
        auto vertex_buffer = vertex_array.GetVertexBufferAt(0);
        
        vertex_array.Bind();
        vertex_buffer->UpdateVertices(sprites.data(), sizeof(SpriteVertex) * sprites.size());

        RenderCommand::DrawTriangles(vertex_array, num_indices_to_draw);

        last_index = 0;
        num_binded_textures = 0;
        num_indices_to_draw = 0;
        sprites.clear();
    }

    void AddSpriteInstance(const std::array<SpriteVertex, NUM_QUAD_VERTICES>& definition, const Transform2D& transform) {
        glm::mat4 transform_matrix = transform.GetTransformMatrix();

        for (const SpriteVertex& sprite_vertex : definition) {
            SpriteVertex vertex = sprite_vertex;
            vertex.position = transform_matrix * glm::vec4(vertex.position, 0, 1);
            sprites.emplace_back(vertex);
        }

        num_indices_to_draw += 6;
    }

    void BindNewTexture(std::shared_ptr<Texture> texture) {
        bind_textures[num_binded_textures++] = texture;
    }
};

static SpriteBatch* sprite_batch = nullptr;
static glm::mat4 projection_{1.0f};

void Renderer2D::Initialize() {
}

void Renderer2D::Quit() {
    SafeDelete(sprite_batch);
}

void Renderer2D::SetDrawShader(const std::shared_ptr<Shader>& shader) {
    sprite_batch = new SpriteBatch(std::make_shared<Material>(shader));
}

void Renderer2D::UpdateProjection(const CameraProjection& projection) {
    projection_ = glm::ortho(0.0f, projection.width, 0.0f, projection.height, -1.0f, 1.0f);
}

void Renderer2D::DrawRect(const glm::vec2& mins, const glm::vec2& maxs, const Transform2D& transform, const RgbaColor& color, int texture_id, glm::vec2 tilling) {

    std::array vertices = {
        SpriteVertex{mins, tilling * glm::vec2{0.0f, 0.0f}, texture_id, color},
        SpriteVertex{glm::vec2{maxs.x, mins.y}, tilling * glm::vec2{1.0f, 0.0f}, texture_id, color},
        SpriteVertex{glm::vec2{maxs.x, maxs.y}, tilling * glm::vec2{1.0f, 1.0f}, texture_id, color},
        SpriteVertex{glm::vec2{mins.x, maxs.y}, tilling * glm::vec2{0.0f, 1.0f}, texture_id, color},
    };

    sprite_batch->AddSpriteInstance(vertices, transform);
}

void Renderer2D::FlushDraw() {
    sprite_batch->FlushDraw(projection_);
} 

int Renderer2D::BindTextureToDraw(const std::shared_ptr<Texture>& texture) {
    sprite_batch->BindNewTexture(texture);
    return sprite_batch->num_binded_textures-1;
}

glm::mat4 Transform2D::GetTransformMatrix() const {
    const glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 rotationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{origin, 0.0f});
    rotationMatrix = rotationMatrix * rotationZ;
    rotationMatrix = glm::translate(rotationMatrix, glm::vec3{-origin, 0.0f});

    // TRS matrix
    return glm::translate(glm::mat4(1.0f), glm::vec3{translation + origin, 0.0f}) * rotationMatrix * glm::scale(glm::mat4(1.0f), glm::vec3{scale, 1});
}
