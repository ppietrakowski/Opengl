#include "sandbox_game_layer.h"

#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

SandboxGameLayer::SandboxGameLayer() :
    m_CameraRotation{glm::vec3{0, 0, 0}},
    m_CameraPosition{0.0f, 0.0f, 0.0f},
    m_SkeletalMesh{"untitled.fbx", std::make_shared<Material>(IShader::LoadShader("skeleton.vert", "textured.frag"))}
{
    m_Shader = IShader::LoadShader("shaders/default.vert", "shaders/default.frag");
    m_Unshaded = IShader::LoadShader("shaders/default.vert", "shaders/Unshaded.frag");

    m_CurrentUsed = m_Shader;
    m_CurrentUsed->Use();
    m_CurrentUsed->SetUniformVec3("u_light_color", glm::vec3{1, 1, 1});
    m_CurrentUsed->SetUniformVec3("u_light_pos", glm::vec3{-1, 0, -5});
    m_Shader->SetUniformVec3("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    m_CurrentUsed->SetUniformVec3("u_material.ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    m_CurrentUsed->SetUniformVec3("u_material.specular", glm::vec3{0.797357, 0.723991, 0.208006});
    m_CurrentUsed->SetUniformFloat("u_material.shininess", 87.2f);

    m_Unshaded->Use();
    m_Unshaded->SetUniformVec3("u_material.diffuse", glm::vec3{1, 0, 0});
    m_CurrentUsed->Use();

    m_Shader->SetUniformMat4("u_projection_view", glm::identity<glm::mat4>());
    m_Shader->SetUniformMat4("u_transform", glm::identity<glm::mat4>());
    glm::vec3 white{1.0f, 1.0f, 1.0f};

    m_Shader->SetUniformVec3("u_material.Tint", white);

    m_WireframeMaterial = std::make_shared<Material>(m_Unshaded);
    m_Material = std::make_shared<Material>(m_Shader);

    m_Material->SetVector3Property("diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});
    m_Material->SetVector3Property("ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    m_Material->SetVector3Property("specular", glm::vec3{0.797357, 0.723991, 0.208006});
    m_Material->SetFloatProperty("shininess", 87.2f);
    m_WireframeMaterial->bUseWireframe = true;
    m_CurrentMaterial = m_Material;
    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    m_StaticMesh = std::make_unique<StaticMesh>("postac.obj", m_Material);

    m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
    m_StaticMeshPosition = {2, 0, -10};
    std::vector<std::string> animations = std::move(m_SkeletalMesh.GetAnimationNames());
    m_SkeletalMesh.SetCurrentAnimation(animations[1]);
    RenderCommand::SetClearColor(RgbaColor{50, 30, 170});

    m_SkeletalMesh.bShouldDrawDebugBounds = true;
}

void SandboxGameLayer::OnUpdate(Duration deltaTime)
{
    float dt = deltaTime.GetAsSeconds();

    if (Input::IsKeyPressed(Keys::kW))
    {
        glm::vec3 worldForward = glm::vec3{0, 0, -1};
        glm::vec3 forward = m_CameraRotation * worldForward * dt * m_MoveSpeed;
        m_CameraPosition += forward;
    }
    else if (Input::IsKeyPressed(Keys::kS))
    {
        glm::vec3 worldBackward = glm::vec3{0, 0, 1};
        glm::vec3 backward = m_CameraRotation * worldBackward * dt * m_MoveSpeed;
        m_CameraPosition += backward;
    }

    if (Input::IsKeyPressed(Keys::kE))
    {
        m_Yaw -= m_YawRotationRate * dt;
        m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
    }
    else if (Input::IsKeyPressed(Keys::kQ))
    {
        m_Yaw += m_YawRotationRate * dt;
        m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
    }

    if (Input::IsKeyPressed(Keys::kY))
    {
        glm::vec3 worldUp = glm::vec3{0, 1, 0};
        m_CameraPosition += m_AscendSpeed * worldUp * dt;
    }
    else if (Input::IsKeyPressed(Keys::kH))
    {
        glm::vec3 worldDown = glm::vec3{0, -1, 0};
        m_CameraPosition += m_AscendSpeed * worldDown * dt;
    }
    m_Duration += deltaTime;
    m_LastDeltaSeconds = deltaTime;

    m_SkeletalMesh.UpdateAnimation((m_Duration - m_StartupTime).GetAsSeconds());
}

void SandboxGameLayer::OnRender(Duration deltaTime)
{
    Renderer::BeginScene(glm::inverse(glm::translate(m_CameraPosition) * glm::mat4_cast(m_CameraRotation)), m_CameraPosition);
    m_CurrentUsed->SetUniformVec3("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});
    m_StaticMesh->Render(*m_CurrentMaterial, glm::translate(glm::identity<glm::mat4>(), m_StaticMeshPosition));

    m_Unshaded->Use();
    m_Unshaded->SetUniformVec3("u_material.diffuse", glm::vec3{1, 0, 0});
    Renderer::DrawDebugBox(m_StaticMesh->GetBBoxMin(), m_StaticMesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), m_StaticMeshPosition));
    Renderer::DrawDebugBox(m_StaticMesh->GetBBoxMin(), m_StaticMesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), m_StaticMeshPosition + glm::vec3{10, 0, 0}));

    Renderer::DrawDebugBox(m_BboxMin, m_BboxMax, glm::translate(glm::identity<glm::mat4>(), glm::vec3{10, 2, 10}));

    m_SkeletalMesh.Draw(glm::translate(glm::vec3{0, -2, -1}) * glm::scale(glm::vec3{0.01f, 0.01f, 0.01f}));
    Renderer::FlushDrawDebug(*m_Unshaded);
    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event)
{
    if (event.Type == EventType::kMouseMoved)
    {
        glm::vec2 delta = event.MouseMove.MousePosition - event.MouseMove.LastMousePosition;
        float dt = m_LastDeltaSeconds.GetAsSeconds();

        m_Yaw -= 10 * delta.x * dt;
        m_Pitch -= 10 * delta.y * dt;

        if (m_Pitch < -89)
        {
            m_Pitch = -89;
        }
        if (m_Pitch > 89)
        {
            m_Pitch = 89;
        }

        m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
        return true;
    }

    if (event.Type == EventType::kMouseButtonPressed)
    {
        m_bSterringEntity = !m_bSterringEntity;
    }

    if (event.Type == EventType::kKeyPressed && event.Key.Code == Keys::kP)
    {
        if (m_CurrentMaterial.get() == m_WireframeMaterial.get())
        {
            m_CurrentMaterial = m_Material;
        }
        else
        {
            m_CurrentMaterial = m_WireframeMaterial;
        }
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame()
{
    static int32_t lastFramerate = 0;
    static int32_t frameNum{0};

    if (frameNum == 2)
    {
        lastFramerate = (lastFramerate + static_cast<int32_t>(1000 / m_LastDeltaSeconds.GetAsMilliseconds())) / 2;
    }
    else
    {
        frameNum++;
    }

    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &m_StaticMeshPosition[0], -10, 10);

    RenderStats stats = RenderCommand::GetRenderStats();

    ImGui::Text("Fps: %i", lastFramerate);
    ImGui::Text("Frame time: %.2f ms", m_LastDeltaSeconds.GetAsMilliseconds());
    ImGui::Text("Drawcalls: %u", stats.NumDrawCalls);
    ImGui::Text("Rendered triangles: %u", stats.NumTriangles);
    ImGui::End();
}
