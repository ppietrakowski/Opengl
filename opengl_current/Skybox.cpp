#include "Skybox.hpp"
#include "Renderer.hpp"
#include "Logging.hpp"

static glm::vec3 SkyboxVertices[] = {
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


Skybox::Skybox(std::shared_ptr<CubeMap> cubeMap, std::shared_ptr<Shader> shader) :
    m_CubeMap(cubeMap),
    m_Shader(shader)
{
    std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(SkyboxVertices, GetContainerSizeInt(SkyboxVertices));

    m_VertexArray.AddVertexBuffer(buffer, std::array{
        VertexAttribute{3, PrimitiveVertexType::Float}
        });

    s_Instance = this;

    ENG_LOG_INFO("Using cubemap {}", cubeMap->GetName());
}

void Skybox::Draw()
{
    // include depth test passes when values are equal to depth buffer's content
    RenderCommand::SetDepthFunc(DepthFunction::LessEqual);

    m_Shader->Use();

    // cut last column (translation), so the skybox will be rendered as it was in center of camera
    glm::mat4 viewWithoutTranslation = Renderer::GetViewMatrix();
    viewWithoutTranslation[3] = glm::vec4{0, 0, 0, 1};

    glm::mat4 projectionView = Renderer::GetProjectionMatrix() * viewWithoutTranslation;

    m_Shader->SetUniform("u_ProjectionView", projectionView);
    m_CubeMap->Bind(0);
    m_Shader->SetSamplerUniform("u_skybox_texture", m_CubeMap, 0);

    RenderCommand::DrawArrays(m_VertexArray, ARRAY_NUM_ELEMENTS(SkyboxVertices));

    // switch back basic depth test
    RenderCommand::SetDepthFunc(DepthFunction::Less);
    m_VertexArray.Unbind();
}
