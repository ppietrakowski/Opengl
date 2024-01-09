#include "sandbox_game_layer.h"

#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

SandboxGameLayer::SandboxGameLayer() :
    m_CameraRotation{glm::vec3{0, 0, 0}},
    m_CameraPosition{0.0f, 0.0f, 0.0f} {
    m_Shader = ResourceManager::GetShader("shaders/default.shd");
    m_Unshaded = ResourceManager::GetShader("shaders/unshaded.shd");
    auto skeletalShader = ResourceManager::GetShader("shaders/skeletal_default.shd");

    ResourceManager::CreateMaterial("shaders/default.shd", "default");

    m_SkeletalMesh = ResourceManager::GetSkeletalMesh("untitled.fbx");
    m_SkeletalMesh->main_material = ResourceManager::CreateMaterial("shaders/skeletal_default.shd", "skeletal1");
    m_CurrentUsed = m_Shader;
    m_CurrentUsed->Use();
    m_CurrentUsed->SetUniform("u_light_color", glm::vec3{1, 1, 1});
    m_CurrentUsed->SetUniform("u_light_pos", glm::vec3{-1, 0, -5});
    m_Shader->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    m_CurrentUsed->SetUniform("u_material.ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    m_CurrentUsed->SetUniform("u_material.specular", glm::vec3{0.797357, 0.723991, 0.208006});
    m_CurrentUsed->SetUniform("u_material.shininess", 87.2f);

    m_Unshaded->Use();
    m_Unshaded->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    m_CurrentUsed->Use();

    m_Shader->SetUniform("u_projection_view", glm::identity<glm::mat4>());
    m_Shader->SetUniform("u_transform", glm::identity<glm::mat4>());
    glm::vec3 white{1.0f, 1.0f, 1.0f};

    m_Shader->SetUniform("u_material.Tint", white);

    m_WireframeMaterial = ResourceManager::CreateMaterial("shaders/unshaded.shd", "wireframe");
    m_Material = ResourceManager::CreateMaterial("shaders/default.shd", "postac_material");

    m_Material->SetVector3Property("diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});
    m_Material->SetVector3Property("ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    m_Material->SetVector3Property("specular", glm::vec3{0.797357, 0.723991, 0.208006});
    m_Material->SetFloatProperty("shininess", 87.2f);
    m_WireframeMaterial->use_wireframe = true;
    m_CurrentMaterial = m_Material;
    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    m_StaticMesh = ResourceManager::GetStaticMesh("postac.obj");
    m_StaticMesh->main_material = m_Material;

    uint32_t i = 0;

    for (const auto& path : m_SkeletalMesh->textures) {
        std::string name = "diffuse" + std::to_string(i + 1);
        auto material = m_SkeletalMesh->main_material;
        material->SetTextureProperty(name.c_str(), ResourceManager::GetTexture2D(path));
        ++i;
    }

    m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
    m_StaticMeshPosition = {2, 0, -10};
    std::vector<std::string> animations = std::move(m_SkeletalMesh->GetAnimationNames());
    RenderCommand::SetClearColor(RgbaColor{50, 30, 170});

    for (int i = 0; i <2; ++i) {
        m_SkeletalMeshActor = m_Level.CreateActor("SkeletalMesh" + std::to_string(i));
        m_SkeletalMeshActor.AddComponent<SkeletalMeshComponent>(m_SkeletalMesh);
        m_SkeletalMeshActor.GetComponent<TransformComponent>().scale = glm::vec3{0.01f, 0.01f, 0.01f};
        m_SkeletalMeshActor.GetComponent<TransformComponent>().position = glm::vec3{0, -2 * i - 2, -i - 1};
    }

    Actor staticMeshActor = m_Level.CreateActor("StaticMeshActor");
    staticMeshActor.AddComponent<StaticMeshComponent>(m_StaticMesh);
    staticMeshActor.GetComponent<TransformComponent>().SetLocalEulerAngles(glm::vec3{0, 90, 0});

    m_InstancedMesh = std::make_shared<InstancedMesh>(*m_StaticMesh);
}

void SandboxGameLayer::Update(Duration deltaTime) {
    float dt = deltaTime.GetAsSeconds();

    if (Input::IsKeyPressed(Keys::kW)) {
        glm::vec3 worldForward = glm::vec3{0, 0, -1};
        glm::vec3 forward = m_CameraRotation * worldForward * dt * m_MoveSpeed;
        m_CameraPosition += forward;
    } else if (Input::IsKeyPressed(Keys::kS)) {
        glm::vec3 worldBackward = glm::vec3{0, 0, 1};
        glm::vec3 backward = m_CameraRotation * worldBackward * dt * m_MoveSpeed;
        m_CameraPosition += backward;
    }

    if (Input::IsKeyPressed(Keys::kE)) {
        m_Yaw -= m_YawRotationRate * dt;
        m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
    } else if (Input::IsKeyPressed(Keys::kQ)) {
        m_Yaw += m_YawRotationRate * dt;
        m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
    }

    if (Input::IsKeyPressed(Keys::kY)) {
        glm::vec3 worldUp = glm::vec3{0, 1, 0};
        m_CameraPosition += m_AscendSpeed * worldUp * dt;
    } else if (Input::IsKeyPressed(Keys::kH)) {
        glm::vec3 worldDown = glm::vec3{0, -1, 0};
        m_CameraPosition += m_AscendSpeed * worldDown * dt;
    }
    m_Duration += deltaTime;
    m_LastDeltaSeconds = deltaTime;

    m_Level.BroadcastUpdate(deltaTime);
}

void SandboxGameLayer::Render(Duration deltaTime) {
    Renderer::BeginScene(glm::inverse(glm::translate(m_CameraPosition) * glm::mat4_cast(m_CameraRotation)), m_CameraPosition);
    m_CurrentUsed->Use();
    m_CurrentUsed->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    m_CurrentUsed->StopUsing();

    for (int i = 0; i < 100; ++i) {
        m_InstancedMesh->QueueDraw(glm::vec3{5.0f * i, 2.0f, 1.0f}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}, 0);
    }
    
    m_InstancedMesh->Draw(glm::identity<glm::mat4>(), *m_StaticMesh->main_material);
    m_Unshaded->Use();
    m_Unshaded->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    Renderer::DrawDebugBox(m_StaticMesh->GetBBoxMin(), m_StaticMesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), m_StaticMeshPosition));
    Renderer::DrawDebugBox(m_StaticMesh->GetBBoxMin(), m_StaticMesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), m_StaticMeshPosition + glm::vec3{10, 0, 0}));

    Renderer::DrawDebugBox(m_BboxMin, m_BboxMax, glm::translate(glm::identity<glm::mat4>(), glm::vec3{10, 2, 10}));
    m_Level.BroadcastRender(deltaTime);
    Renderer::FlushDrawDebug(*m_Unshaded);
    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event) {
    if (event.type == EventType::kMouseMoved) {
        glm::vec2 delta = event.mouse_move.mouse_position - event.mouse_move.last_mouse_position;
        float dt = m_LastDeltaSeconds.GetAsSeconds();

        m_Yaw -= m_YawRotationRate * delta.x * dt;
        m_Pitch -= m_PitchRotationRate * delta.y * dt;

        if (m_Pitch < -89) {
            m_Pitch = -89;
        }
        if (m_Pitch > 89) {
            m_Pitch = 89;
        }

        m_CameraRotation = glm::quat{glm::radians(glm::vec3{m_Pitch, m_Yaw, 0.0f})};
        return true;
    }

    if (event.type == EventType::kMouseButtonPressed) {
        DO_ONCE([this]() {
            Actor actor = m_Level.FindActor("StaticMeshActor");
            actor.DestroyActor();
        }
        );

        m_bSterringEntity = !m_bSterringEntity;
    }

    if (event.type == EventType::kKeyPressed && event.key_event.code == Keys::kP) {
        if (m_CurrentMaterial.get() == m_WireframeMaterial.get()) {
            m_CurrentMaterial = m_Material;
        } else {
            m_CurrentMaterial = m_WireframeMaterial;
        }
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame() {
    static std::int32_t lastFramerate = 0;
    static std::int32_t frameNum{0};

    if (frameNum == 2) {
        lastFramerate = (lastFramerate + static_cast<std::int32_t>(1000 / m_LastDeltaSeconds.GetAsMilliseconds())) / 2;
    } else {
        frameNum++;
    }

    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &m_StaticMeshPosition[0], -10, 10);

    RenderStats stats = RenderCommand::GetRenderStats();

    ImGui::Text("Fps: %i", lastFramerate);
    ImGui::Text("Frame time: %.2f ms", m_LastDeltaSeconds.GetAsMilliseconds());
    ImGui::Text("Drawcalls: %u", stats.num_drawcalls);
    ImGui::Text("Rendered triangles: %u", stats.num_drawn_triangles);
    ImGui::End();
}
