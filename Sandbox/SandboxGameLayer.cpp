#include "SandboxGameLayer.h"

#include "Renderer.h"
#include "Imgui/imgui.h"

static glm::vec3 BoxVertices[] =
{
    {-1, 1, 1},
    {1, 1, 1},
    {1, -1, 1},
    {-1, -1, 1},
    {1, -1, -1}, // 4
    {1, 1, -1}, // 5
    {-1, -1, -1}, // 6
    {-1, 1, -1}, // 7
};

static unsigned int BoxIndices[] =
{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

SandboxGameLayer::SandboxGameLayer() :
    _cameraRotation{ glm::vec3{0, 0, 0} },
    _cameraPosition{ 0.0f, 0.0f, 0.0f }
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

    glm::mat4 identity{ 1.0f };
    _shader->SetUniformMat4("u_ProjectionView", identity);
    _shader->SetUniformMat4("u_Transform", identity);
    glm::vec3 white{ 1.0f, 1.0f, 1.0f };

    _shader->SetUniformVec3("u_Material.Tint", white);

    _wireframeMaterial = std::make_shared<Material>(_unshaded);
    _material = std::make_shared<Material>(_shader);

    _material->SetVector3Property("diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});
    _material->SetVector3Property("ambient", glm::vec3{ 0.01f, 0.01f, 0.01f });
    _material->SetVector3Property("specular", glm::vec3{ 0.797357, 0.723991, 0.208006 });
    _material->SetFloatProperty("shininess", 87.2f);

    _mesh = std::make_unique<StaticMesh>("postac.obj", _material);

    _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    _position = { 2, 0, -10 };

    VertexBuffer buffer(BoxVertices, sizeof(BoxVertices), true);
    IndexBuffer indexBuffer(BoxIndices, sizeof(BoxIndices) / sizeof(BoxIndices[0]), true);

    _debugVertexArray = std::make_unique<VertexArray>();
    VertexAttribute attributes[] = { {3, EPrimitiveVertexType::Float} };

    _debugVertexArray->AddBuffer(std::move(buffer), attributes);
    _debugVertexArray->SetIndexBuffer(std::move(indexBuffer));

    RenderCommand::SetClearColor(0.2f, 0.3f, 0.6f);

    glm::vec3 boxmin = glm::vec4{ _mesh->GetBBoxMin(), 1.0f } *glm::translate(glm::mat4{ 1.0f }, _position);
    glm::vec3 boxmax = glm::vec4{ _mesh->GetBBoxMax(), 1.0f } *glm::translate(glm::mat4{ 1.0f }, _position);

    glm::vec3 vertices[] = {
        {boxmin[0], boxmin[1], boxmin[2]},
        {boxmax[0], boxmin[1], boxmin[2]},
        {boxmax[0], boxmax[1], boxmin[2]},
        {boxmin[0], boxmax[1], boxmin[2]},
        
        {boxmin[0], boxmin[1], boxmax[2]},
        {boxmax[0], boxmin[1], boxmax[2]},
        {boxmax[0], boxmax[1], boxmax[2]},
        {boxmin[0], boxmax[1], boxmax[2]}
    };

    VertexBuffer& vb = _debugVertexArray->GetVertexBufferAt(0);
    vb.UpdateVertices(&vertices[0], sizeof(vertices));
}

void SandboxGameLayer::OnUpdate(float deltaTime)
{
    if (IsKeyDown(GLFW_KEY_W))
    {
        glm::vec3 forward = _cameraRotation * glm::vec3{ 0, 0, -1 } *deltaTime * 10.0f;
        _cameraPosition += forward;
    }
    else if (IsKeyDown(GLFW_KEY_S))
    {
        glm::vec3 backward = _cameraRotation * glm::vec3{ 0, 0, 1 } *deltaTime * 10.0f;
        _cameraPosition += backward;
    }

    if (IsKeyDown(GLFW_KEY_P) && !_wasPressedLastTime)
    {
        RenderCommand::ToggleWireframe();
        _currentUsed = _unshaded;
        _wasPressedLastTime = true;
    }
    else if (_wasPressedLastTime)
    {
        _currentUsed = _shader;
        _wasPressedLastTime = false;
    }
    
    if (IsKeyDown(GLFW_KEY_E))
    {
        _yaw -= 10 * 4 * _lastDeltaSeconds;
        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    }
    else if (IsKeyDown(GLFW_KEY_Q))
    {
        _yaw += 10 * 4 * _lastDeltaSeconds;
        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    }

    if (IsKeyDown(GLFW_KEY_Y))
    {
        _cameraPosition += glm::vec3{ 0, 10, 0 } * deltaTime;
    }
    else if (IsKeyDown(GLFW_KEY_H))
    {
        _cameraPosition += glm::vec3{ 0, -10, 0 } *deltaTime;
    }

    _lastDeltaSeconds = deltaTime;
}

void SandboxGameLayer::OnRender(float deltaTime)
{
    Renderer::BeginScene(glm::inverse(glm::translate(glm::mat4{ 1.0f }, _cameraPosition) * glm::mat4_cast(_cameraRotation)), _cameraPosition);
    _currentUsed->SetUniformVec3("u_Material.diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });
    _mesh->Render(glm::translate(glm::mat4{ 1.0f }, _position));

    glLineWidth(2);

    _unshaded->Use();
    Renderer::AddDebugBox(_mesh->GetBBoxMin(), _mesh->GetBBoxMax(), glm::translate(glm::mat4{ 1.0f }, _position));

    Renderer::FlushDrawDebug(*_unshaded);
    Renderer::EndScene();
    glLineWidth(1);
}

bool SandboxGameLayer::OnEvent(const Event& event)
{
    if (0 && event.Type == EEventType::MouseMoved)
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

    if (event.Type == EEventType::MouseButtonPressed)
    {
        _sterringEntity = !_sterringEntity;
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame()
{
    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &_position[0], -10, 10);
    ImGui::End();
}
