#include "renderer_2d.h"

struct SpriteBatch {
    BatchBase<SpriteVertex> batch_base{
        std::array{
            VertexAttribute{2, PrimitiveVertexType::kFloat},
            VertexAttribute{2, PrimitiveVertexType::kFloat},
            VertexAttribute{1, PrimitiveVertexType::kInt},
            VertexAttribute{1, PrimitiveVertexType::kUnsignedInt}
        }
    };

    std::array<std::shared_ptr<Texture>, kMinTextureUnits> bind_textures;
    int last_index = 0;
};

static SpriteBatch* sprite_batch = nullptr;
static std::shared_ptr<Shader> shader_2d = nullptr;
static std::shared_ptr<Material> material_2d = nullptr;
static glm::mat4 projection_{1.0f};

void Renderer2D::Initialize() {
    sprite_batch = new SpriteBatch();
}

void Renderer2D::Quit() {
    SafeDelete(sprite_batch);

}

void Renderer2D::SetDrawShader(const std::shared_ptr<Shader>& shader) {
    shader_2d = shader;
    material_2d = std::make_shared<Material>(shader);
}

void Renderer2D::UpdateProjection(const CameraProjection& projection) {
    projection_ = glm::ortho(0.0f, projection.width, 0.0f, projection.height);
}

void Renderer2D::DrawRect(const glm::vec2& mins, const glm::vec2& maxs, const Transform2D& transform, const RgbaColor& color, int texture_id) {

    std::array vertices = {
        SpriteVertex{mins, glm::vec2{0.0f, 1.0f}, texture_id, color},
        SpriteVertex{glm::vec2{maxs.x, mins.y}, glm::vec2{1.0f, 1.0f}, texture_id, color},
        SpriteVertex{glm::vec2{maxs.x, maxs.y}, glm::vec2{1.0f, 0.0f}, texture_id, color},
        SpriteVertex{glm::vec2{mins.x, maxs.y}, glm::vec2{0.0f}, texture_id, color},
    };

    uint32_t indices[] = {0, 2, 1, 3, 2, 0};
    sprite_batch->batch_base.QueueDraw(BatchGeometryInfo<SpriteVertex>{vertices, indices}, transform);
}

void Renderer2D::FlushDraw() {
    sprite_batch->batch_base.DrawTriangles(glm::mat4{1.0f}, *material_2d);
}

int Renderer2D::BindTextureToDraw(const std::shared_ptr<Texture>& texture) {
    sprite_batch->bind_textures[sprite_batch->last_index++] = texture;
    return sprite_batch->last_index - 1;
}
