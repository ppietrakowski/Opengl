#include "sandbox_game_layer.h"

#include <functional>
#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

static void SetupDefaultProperties(const std::shared_ptr<Material>& material)
{
    material->SetFloatProperty("reflection_factor", 0.1f);
    material->SetFloatProperty("shininess", 32.0f);
    material->bTransparent = true;
    material->bCullFaces = false;
}

struct FpsCounter
{
    float AccumulatedFrameTime = 1.0f;
    milliseconds_float_t FrameTime = milliseconds_float_t::zero();
    FpsCounter()
    {
    }

    double GetFrameTime() const
    {
        return FrameTime.count();
    }

    void Tick(Duration deltaSeconds)
    {
        AccumulatedFrameTime += deltaSeconds.GetSeconds();

        if (AccumulatedFrameTime >= 1.0f)
        {
            AccumulatedFrameTime = 0.0f;
            FrameTime = deltaSeconds.GetChronoNanoSeconds();
        }
    }
};

DECLARE_COMPONENT_TICKABLE(FpsCounter);

SandboxGameLayer::SandboxGameLayer(Game* game) :
    m_Game(game)
{
    m_DefaultShader = ResourceManager::GetShader("assets/shaders/default.shd");
    m_DebugShader = ResourceManager::GetShader("assets/shaders/unshaded.shd");

    ResourceManager::CreateMaterial("assets/shaders/default.shd", "default");
    InitializeSkeletalMesh();

    m_CurrentUsedShader = m_DefaultShader;

    Renderer::InitializeDebugDraw(m_DebugShader);
    Renderer2D::SetDrawShader(ResourceManager::GetShader("assets/shaders/sprite_2d.shd"));

    auto defaultMaterial = ResourceManager::CreateMaterial("assets/shaders/default.shd", "postac_material");
    defaultMaterial->SetFloatProperty("shininess", 32.0f);

    std::shared_ptr<StaticMesh> staticMesh = ResourceManager::GetStaticMesh("assets/cube.obj");
    staticMesh->MainMaterial = defaultMaterial;
    defaultMaterial->bTransparent = true;

    CreateSkeletalActors();

    std::shared_ptr<Material> instancedMeshMaterial = ResourceManager::CreateMaterial("assets/shaders/instanced.shd", "instanced");
    SetupDefaultProperties(instancedMeshMaterial);
    m_InstancedMesh = std::make_shared<InstancedMesh>(staticMesh, instancedMeshMaterial);

    CreateInstancedMeshActor("assets/cube.obj", instancedMeshMaterial);

    PlaceLightsAndPlayer();

    std::array<std::string, 6> texturePaths = {
        "assets/skybox/right.jpg",
        "assets/skybox/left.jpg",
        "assets/skybox/top.jpg",
        "assets/skybox/bottom.jpg",
        "assets/skybox/front.jpg",
        "assets/skybox/back.jpg"
    };

    m_Skybox = std::make_unique<Skybox>(std::make_shared<CubeMap>(texturePaths), ResourceManager::GetShader("assets/shaders/skybox.shd"));
}

void SandboxGameLayer::Update(Duration deltaTime)
{
    float dt = deltaTime.GetSeconds();

    auto shader = ResourceManager::GetShader("assets/shaders/instanced.shd");
    shader->Use();
    m_LastDeltaSeconds = deltaTime;

    m_Level.BroadcastUpdate(deltaTime);
}

void SandboxGameLayer::Render()
{
    Renderer::BeginScene(m_Level.CameraPosition, m_Level.CameraRotation);

    glm::vec3 lightPosition{0.0f};

    {
        Actor directional_light = m_Level.FindActor("directional_light");
        lightPosition = directional_light.GetTransform().Position;
    }

    m_CurrentUsedShader->Use();
    m_CurrentUsedShader->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    m_DebugShader->Use();
    m_DebugShader->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    Actor characterActor;

    if (m_Level.TryFindActor("SkeletalMesh0", characterActor))
    {
        Transform transform = characterActor.GetTransform().GetAsTransform();
        transform.Scale = {1, 1, 1};
        Renderer::DrawDebugBox(m_TestSkeletalMesh->GetBoundingBox(), transform);
    }

    // draw directional light gizmo
    for (int i = 0; i < 3; ++i)
    {
        glm::vec3 pos = glm::vec3{0.0f};
        glm::vec4 color = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
        pos[i] = 1;
        color[i] = 1;

        Line line{lightPosition, lightPosition + pos};

        Renderer::DrawDebugLine(line, Transform{}, color);
    }

    Renderer::DrawDebugLine(Line{lightPosition, lightPosition + 2.0f * glm::vec3{0, -1, 0}}, Transform{}, glm::vec4(0, 0, 1, 1));

    m_Level.BroadcastRender();
    m_Skybox->Draw();
    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event)
{
    if (event.Type == EventType::KeyPressed && event.Key.Key == GLFW_KEY_P)
    {
        m_Game->SetMouseVisible(!m_Game->IsMouseVisible());
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame()
{
    {
        FpsCounter& counter = m_Level.FindActor("player").GetComponent<FpsCounter>();

        RenderStats stats = RenderCommand::GetRenderStats();

        ImGui::Begin("Stats");
        ImGui::Text("Fps: %i", static_cast<int>(round(1000.0 / counter.GetFrameTime())));
        ImGui::Text("Frame time: %.2f ms", counter.GetFrameTime());
        ImGui::Text("Drawcalls: %i", stats.NumDrawcalls);
        std::string text = FormatSize(stats.IndexBufferMemoryAllocation);
        ImGui::Text("NumIndicesMemoryAllocated: %s", text.c_str());
        text = FormatSize(stats.VertexBufferMemoryAllocation);
        ImGui::Text("NumVerticesMemoryAllocated: %s", text.c_str());
        text = FormatSize(UniformBuffer::s_NumBytesAllocated);
        ImGui::Text("NumBytesUniformBuffer: %s", text.c_str());
        text = FormatSize(Texture2D::s_NumTextureVramUsed);
        ImGui::Text("NumTextureMemoryUsage: %s", text.c_str());
        ImGui::End();
    }

    Actor actorToRemove{};
    std::string removedActorName;

    ImGui::Begin("Hierarchy");

    for (auto& [name, actor] : m_Level)
    {
        if (ImGui::Button(name.c_str()))
        {
            m_SelectedActor = actor;
            break;
        }

        ImGui::SameLine();

        ImGui::PushID(name.c_str());
        if (ImGui::Button("X"))
        {
            actorToRemove = actor;
            removedActorName = name;
        }
        ImGui::PopID();

        ImGui::NextColumn();
    }

    ImGui::End();

    if (removedActorName != "player" && !removedActorName.empty())
    {
        actorToRemove.DestroyActor();
    }
}

void SandboxGameLayer::OnImgizmoFrame()
{
    if (!m_SelectedActor.IsAlive())
    {
        return;
    }

    glm::mat4 transform = m_SelectedActor.GetTransform().GetWorldTransformMatrix();

    if (ImGuizmo::Manipulate(glm::value_ptr(Renderer::s_View), glm::value_ptr(Renderer::s_Projection),
        ImGuizmo::OPERATION::UNIVERSAL, ImGuizmo::WORLD, glm::value_ptr(transform)
    ))
    {

        glm::vec3 pos, rot, scale;

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), &pos[0], &rot[0], &scale[0]);

        TransformComponent& transform = m_SelectedActor.GetTransform();
        transform.SetEulerAngles(rot);
        transform.Position = pos;
        transform.Scale = scale;
    }
}

void SandboxGameLayer::MoveForward(Actor& player, float axisValue)
{
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 forward = axisValue * transform.GetForwardVector() * static_cast<float>(m_LastDeltaSeconds.GetSeconds()) * m_MoveSpeed;
    transform.Translate(forward);
}

void SandboxGameLayer::MoveRight(Actor& player, float axisValue)
{
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 right = axisValue * transform.GetRightVector() * static_cast<float>(m_LastDeltaSeconds.GetSeconds()) * m_MoveSpeed;
    transform.Translate(right);
}

void SandboxGameLayer::RotateCamera(Actor& player, glm::vec2 mouseMoveDelta)
{
    if (m_Game->IsMouseVisible())
    {
        return;
    }

    float dt = m_LastDeltaSeconds.GetSeconds();

    TransformComponent& transform = player.GetComponent<TransformComponent>();

    m_CameraYaw -= m_YawRotationRate * mouseMoveDelta.x * dt;
    m_CameraPitch -= m_PitchRotationRate * mouseMoveDelta.y * dt;

    if (m_CameraPitch < -89)
    {
        m_CameraPitch = -89;
    }
    if (m_CameraPitch > 89)
    {
        m_CameraPitch = 89;
    }

    transform.SetEulerAngles(m_CameraPitch, m_CameraYaw, 0.0f);
}

void SandboxGameLayer::InitializeSkeletalMesh()
{
    auto skeletal_shader = ResourceManager::GetShader("assets/shaders/skeletal_default.shd");
    m_TestSkeletalMesh = ResourceManager::GetSkeletalMesh("assets/test_character.fbx");
    m_TestSkeletalMesh->MainMaterial = ResourceManager::CreateMaterial("assets/shaders/skeletal_default.shd", "skeletal1");

    for (int i = 0; i < m_TestSkeletalMesh->TextureNames.size(); ++i)
    {
        std::string property_name = std::string{"diffuse"} + std::to_string(i + 1);
        m_TestSkeletalMesh->MainMaterial->SetTextureProperty(property_name.c_str(), ResourceManager::GetTexture2D(m_TestSkeletalMesh->TextureNames[i]));
    }

    std::shared_ptr<Material> material = m_TestSkeletalMesh->MainMaterial;

    material->bCullFaces = false;
    material->SetFloatProperty("shininess", 32.0f);
}

void SandboxGameLayer::CreateSkeletalActors()
{
    for (std::int32_t i = 0; i < 2; ++i)
    {
        Actor skeletalMeshActor = m_Level.CreateActor("SkeletalMesh" + std::to_string(i));
        skeletalMeshActor.AddComponent<SkeletalMeshComponent>(m_TestSkeletalMesh);
        skeletalMeshActor.GetComponent<TransformComponent>().Scale = glm::vec3{0.01f, 0.01f, 0.01f};
        skeletalMeshActor.GetComponent<TransformComponent>().Position = glm::vec3{0, -2 * i - 2, -i - 1};
    }
}

Actor SandboxGameLayer::CreateInstancedMeshActor(const std::string& filePath, const std::shared_ptr<Material>& material)
{
    Actor instanceMesh = m_Level.CreateActor("InstancedMesh");
    instanceMesh.AddComponent<InstancedMeshComponent>(ResourceManager::GetStaticMesh(filePath), material);

    material->SetTextureProperty("diffuse1", ResourceManager::GetTexture2D("assets/T_Metal_Steel_D.TGA"));

    InstancedMeshComponent& instancedMesh = instanceMesh.GetComponent<InstancedMeshComponent>();

    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 400; ++j)
        {
            Transform transform{glm::vec3{5.0f * i, 2.0f, 3.0f * j}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}};
            instancedMesh.AddInstance(transform);
        }
    }

    return instanceMesh;
}

void SandboxGameLayer::PlaceLightsAndPlayer()
{
    Actor lightActor = m_Level.CreateActor("point_light");

    lightActor.AddComponent<SpotLightComponent>();

    SpotLightComponent& spotLight = lightActor.GetComponent<SpotLightComponent>();

    spotLight.DirectionLength = 40.0f;
    spotLight.Direction = glm::vec3{0, -1, 0};
    spotLight.Color = glm::vec3{1, 0, 0};
    spotLight.CutOffAngle = 45.0f;
    lightActor.GetTransform().Position = glm::vec3{4, 5, 0};

    Actor player = m_Level.CreateActor("player");
    player.AddComponent<PlayerController>(player);
    player.AddComponent<CameraComponent>();
    player.AddComponent<FpsCounter>();

    Actor directionalLight = m_Level.CreateActor("directional_light");
    m_SelectedActor = directionalLight;
    directionalLight.AddComponent<DirectionalLightComponent>();

    DirectionalLightComponent& directional = directionalLight.GetComponent<DirectionalLightComponent>();
    directional.Direction = {0, -1, 0};

    PlayerController& controller = player.GetComponent<PlayerController>();
    controller.BindForwardCallback(std::bind(&SandboxGameLayer::MoveForward, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindRightCallback(std::bind(&SandboxGameLayer::MoveRight, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindMouseMoveCallback(std::bind(&SandboxGameLayer::RotateCamera, this, std::placeholders::_1, std::placeholders::_2));

}
