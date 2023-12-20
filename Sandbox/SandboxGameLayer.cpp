#include "SandboxGameLayer.h"

#include "Renderer.h"
#include "Imgui/imgui.h"
#include "ErrorMacros.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Logging.h"

SandboxGameLayer::SandboxGameLayer() :
    _cameraRotation{ glm::vec3{0, 0, 0} },
    _cameraPosition{ 0.0f, 0.0f, 0.0f },
    mesh{"untitled.fbx", std::make_shared<Material>(Shader::LoadShader("skeleton.vert", "textured.frag"))}
{
    _shader = Shader::LoadShader("shaders/default.vert", "shaders/default.frag");
    _unshaded = Shader::LoadShader("shaders/default.vert", "shaders/Unshaded.frag");

    _currentUsed = _shader;
    _currentUsed->Use();
    _currentUsed->SetUniformVec3("u_LightColor", glm::vec3{ 1, 1, 1 });
    _currentUsed->SetUniformVec3("u_LightPos", glm::vec3{ -1, 0, -5 });
    _shader->SetUniformVec3("u_Material.diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });

    _currentUsed->SetUniformVec3("u_Material.ambient", glm::vec3{ 0.01f, 0.01f, 0.01f });
    _currentUsed->SetUniformVec3("u_Material.specular", glm::vec3{ 0.797357, 0.723991, 0.208006 });
    _currentUsed->SetUniformFloat("u_Material.shininess", 87.2f);

    _unshaded->Use();
    _unshaded->SetUniformVec3("u_Material.diffuse", glm::vec3{ 1, 0, 0 });
    _currentUsed->Use();

    _shader->SetUniformMat4("u_ProjectionView", glm::identity<glm::mat4>());
    _shader->SetUniformMat4("u_Transform", glm::identity<glm::mat4>());
    glm::vec3 white{ 1.0f, 1.0f, 1.0f };

    _shader->SetUniformVec3("u_Material.Tint", white);

    _wireframeMaterial = std::make_shared<Material>(_unshaded);
    _material = std::make_shared<Material>(_shader);

    _material->SetVector3Property("diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });
    _material->SetVector3Property("ambient", glm::vec3{ 0.01f, 0.01f, 0.01f });
    _material->SetVector3Property("specular", glm::vec3{ 0.797357, 0.723991, 0.208006 });
    _material->SetFloatProperty("shininess", 87.2f);
    _wireframeMaterial->UseWireframe = true;
    _currentMaterial = _material;

    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    _mesh = std::make_unique<StaticMesh>("postac.obj", _material);

    _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    _position = { 2, 0, -10 };

    std::vector<std::string> animations = std::move(mesh.GetAnimationNames()) ;
    RenderCommand::SetClearColor(0.2f, 0.3f, 0.6f);
}

void SandboxGameLayer::OnUpdate(float deltaTime)
{
    if (IsKeyDown(GLFW_KEY_W))
    {
        glm::vec3 worldForward = glm::vec3{ 0, 0, -1 };
        glm::vec3 forward = _cameraRotation * worldForward * deltaTime * _moveSpeed;
        _cameraPosition += forward;
    }
    else if (IsKeyDown(GLFW_KEY_S))
    {
        glm::vec3 worldBackward = glm::vec3{ 0, 0, 1 };
        glm::vec3 backward = _cameraRotation * worldBackward * deltaTime * _moveSpeed;
        _cameraPosition += backward;
    }

    if (IsKeyDown(GLFW_KEY_E))
    {
        _yaw -= _yawRotationRate * _lastDeltaSeconds;
        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    }
    else if (IsKeyDown(GLFW_KEY_Q))
    {
        _yaw += _yawRotationRate * _lastDeltaSeconds;
        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    }

    if (IsKeyDown(GLFW_KEY_Y))
    {
        glm::vec3 worldUp = glm::vec3{ 0, 1, 0 };
        _cameraPosition += _ascendSpeed * worldUp * deltaTime;
    }
    else if (IsKeyDown(GLFW_KEY_H))
    {
        glm::vec3 worldDown = glm::vec3{ 0, -1, 0 };
        _cameraPosition += _ascendSpeed * worldDown * deltaTime;
    }

    _lastDeltaSeconds = deltaTime;
}

void SandboxGameLayer::OnRender(float deltaTime)
{
    Renderer::BeginScene(glm::inverse(glm::translate(glm::identity<glm::mat4>(), _cameraPosition) * glm::mat4_cast(_cameraRotation)), _cameraPosition);
    _currentUsed->SetUniformVec3("u_Material.diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });
    _mesh->Render(*_currentMaterial, glm::translate(glm::identity<glm::mat4>(), _position));

    glLineWidth(2);

    _unshaded->Use();
    _unshaded->SetUniformVec3("u_Material.diffuse", glm::vec3{ 1, 0, 0 });
    Renderer::AddDebugBox(_mesh->GetBBoxMin(), _mesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), _position));
    Renderer::AddDebugBox(_mesh->GetBBoxMin(), _mesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), _position + glm::vec3{ 10, 0, 0 }));

    Renderer::AddDebugBox(BboxMin, BboxMax, glm::translate(glm::identity<glm::mat4>(), glm::vec3{ 10, 2, 10 }));
    Renderer::FlushDrawDebug(*_unshaded);
    mesh.Draw(glm::scale(glm::vec3{ 0.01f, 0.01f, 0.01f }));

    Renderer::EndScene();
    glLineWidth(1);
}

bool SandboxGameLayer::OnEvent(const Event& event)
{
    if (event.Type == EventType::MouseMoved)
    {
        glm::vec2 delta = event.MouseMove.MousePosition - event.MouseMove.LastMousePosition;

        _yaw -= 10 * delta.x * _lastDeltaSeconds;
        _pitch -= 10 * delta.y * _lastDeltaSeconds;

        if (_pitch < -89)
        {
            _pitch = -89;
        }
        if (_pitch > 89)
        {
            _pitch = 89;
        }

        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
        return true;
    }

    if (event.Type == EventType::MouseButtonPressed)
    {
        _sterringEntity = !_sterringEntity;
    }

    if (event.Type == EventType::KeyPressed && event.Key.Code == GLFW_KEY_P)
    {
        if (_currentMaterial.get() == _wireframeMaterial.get())
        {
            _currentMaterial = _material;
        }
        else
        {
            _currentMaterial = _wireframeMaterial;
        }
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame()
{
    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &_position[0], -10, 10);
    ImGui::End();
}
