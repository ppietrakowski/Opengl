#include "skybox.h"
#include "renderer.h"

static glm::vec3 kSkyboxVertices[] = {
    // positions          
    glm::vec3{-1.0f,  1.0f, -1.0f}, 
    glm::vec3{-1.0f, -1.0f, -1.0f}, 
    glm::vec3{ 1.0f, -1.0f, -1.0f}, 
    glm::vec3{ 1.0f, -1.0f, -1.0f}, 
    glm::vec3{ 1.0f,  1.0f, -1.0f}, 
    glm::vec3{-1.0f,  1.0f, -1.0f}, 

    glm::vec3{-1.0f, -1.0f,  1.0f},
    glm::vec3{-1.0f, -1.0f, -1.0f},
    glm::vec3{-1.0f,  1.0f, -1.0f},
    glm::vec3{-1.0f,  1.0f, -1.0f},
    glm::vec3{-1.0f,  1.0f,  1.0f},
    glm::vec3{-1.0f, -1.0f,  1.0f},

    glm::vec3{1.0f, -1.0f, -1.0f},
    glm::vec3{1.0f, -1.0f,  1.0f},
    glm::vec3{1.0f,  1.0f,  1.0f},
    glm::vec3{1.0f,  1.0f,  1.0f},
    glm::vec3{1.0f,  1.0f, -1.0f},
    glm::vec3{1.0f, -1.0f, -1.0f},

    glm::vec3{-1.0f, -1.0f,  1.0f},
    glm::vec3{-1.0f,  1.0f,  1.0f},
    glm::vec3{ 1.0f,  1.0f,  1.0f},
    glm::vec3{ 1.0f,  1.0f,  1.0f},
    glm::vec3{ 1.0f, -1.0f,  1.0f},
    glm::vec3{-1.0f, -1.0f,  1.0f},

    glm::vec3{-1.0f,  1.0f, -1.0f},
    glm::vec3{ 1.0f,  1.0f, -1.0f},
    glm::vec3{ 1.0f,  1.0f,  1.0f},
    glm::vec3{ 1.0f,  1.0f,  1.0f},
    glm::vec3{-1.0f,  1.0f,  1.0f},
    glm::vec3{-1.0f,  1.0f, -1.0f},

    glm::vec3{-1.0f, -1.0f, -1.0f},
    glm::vec3{-1.0f, -1.0f,  1.0f},
    glm::vec3{ 1.0f, -1.0f, -1.0f},
    glm::vec3{ 1.0f, -1.0f, -1.0f},
    glm::vec3{-1.0f, -1.0f,  1.0f},
    glm::vec3{ 1.0f, -1.0f,  1.0f}
};

Skybox::Skybox(const std::shared_ptr<CubeMap>& cube_map, const std::shared_ptr<Shader>& shader) :
    cube_map_(cube_map),
    shader_(shader) {
    vertex_array_ = std::make_shared<VertexArray>();

    std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(kSkyboxVertices, sizeof(kSkyboxVertices));

    vertex_array_->AddVertexBuffer(buffer, std::array{
        VertexAttribute{3, PrimitiveVertexType::kFloat}
        });
}

void Skybox::Draw() {
    // include depth test passes when values are equal to depth buffer's content
    RenderCommand::SetDepthFunc(DepthFunction::LessEqual);

    shader_->Use();

    // cut last column (translation), so the skybox will be rendered as it was in center of camera
    glm::mat4 view_without_translation = Renderer::view_;
    view_without_translation[3] = glm::vec4{0, 0, 0, 1};

    shader_->SetUniform("u_view", view_without_translation);
    shader_->SetUniform("u_projection", Renderer::projection_);
    cube_map_->Bind(0);
    shader_->SetSamplerUniform("u_skybox_texture", cube_map_, 0);

    RenderCommand::SetWireframe(false);
    RenderCommand::DrawTrianglesArrays(*vertex_array_, ARRAY_NUM_ELEMENTS(kSkyboxVertices));

    // switch back basic depth test
    RenderCommand::SetDepthFunc(DepthFunction::Less);
}
