#include "sandbox_game_layer.h"

#include <functional>
#include "Renderer.hpp"
#include "Imgui/imgui.h"
#include "ErrorMacros.hpp"
#include "Logging.hpp"

#include <glm/gtx/matrix_decompose.hpp>
#include <random>

static void SetupDefaultProperties(const std::shared_ptr<Material>& material)
{
    material->SetFloatProperty("ReflectionFactor", 0.05f);
    material->SetFloatProperty("Shininess", 32.0f);
}

#define TEXTURE_ID_TO_IMGUI_TEXTURE_ID(Id) reinterpret_cast<void*>(static_cast<intptr_t>(Id))

class MaterialGuiDisplay : public IMaterialParameterVisitor
{
    // Inherited via IMaterialParameterVisitor
    void Visit(PrimitiveParameter<int>& param, const std::string& name) override
    {
    }

    void Visit(PrimitiveParameter<float>& param, const std::string& name) override
    {
        float value = param.GetPrimitiveValue();
        if (ImGui::InputFloat(name.c_str(), &value, 0.1f))
        {
            param.SetValue(value);
        }
    }

    void Visit(PrimitiveParameter<glm::vec2>& param, const std::string& name) override
    {
        glm::vec2 value = param.GetPrimitiveValue();
        if (ImGui::InputFloat2(name.c_str(), &value[0]))
        {
            param.SetValue(value);
        }
    }

    void Visit(PrimitiveParameter<glm::vec3>& param, const std::string& name) override
    {
        glm::vec3 value = param.GetPrimitiveValue();
        if (ImGui::InputFloat3(name.c_str(), &value[0]))
        {
            param.SetValue(value);
        }
    }

    void Visit(PrimitiveParameter<glm::vec4>& param, const std::string& name) override
    {
        glm::vec4 value = param.GetPrimitiveValue();
        if (ImGui::ColorEdit4(name.c_str(), &value[0]))
        {
            param.SetValue(value);
        }
    }

    void Visit(TextureParameter& param, const std::string& name) override
    {
        std::shared_ptr<ITexture> texture = param.GetPrimitiveValue();

        ImGui::Text("%s", name.c_str());
        ImGui::Image(TEXTURE_ID_TO_IMGUI_TEXTURE_ID(texture->GetRendererId()), ImVec2(300, 300));
    }
};

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

SandboxGameLayer::SandboxGameLayer(std::shared_ptr<Game> game) :
    m_Game(game),
    m_Skybox(std::make_shared<CubeMap>(std::array<std::string, 6> {
    "assets/skybox/right.jpg",
        "assets/skybox/left.jpg",
        "assets/skybox/top.jpg",
        "assets/skybox/bottom.jpg",
        "assets/skybox/front.jpg",
        "assets/skybox/back.jpg"
    }), ResourceManager::GetShader("assets/shaders/skybox.shd")),
    m_GuiDisplayVisitor(std::make_unique<MaterialGuiDisplay>())
{
    m_Level = game->GetCurrentLevel();

    auto defaultShader = ResourceManager::GetShader("assets/shaders/default.shd");
    auto debugShader = ResourceManager::GetShader("assets/shaders/unshaded.shd");

    ResourceManager::CreateMaterial("assets/shaders/default.shd", "default");
    InitializeSkeletalMesh();

    auto defaultMaterial = ResourceManager::CreateMaterial("assets/shaders/default.shd", "postac_material");
    defaultMaterial->SetFloatProperty("Shininess", 32.0f);

    std::shared_ptr<StaticMesh> staticMesh = ResourceManager::GetStaticMesh("assets/box.fbx");
    staticMesh->SetMaterial(defaultMaterial);

    CreateSkeletalActors();

    std::shared_ptr<Material> instancedMeshMaterial = ResourceManager::CreateMaterial("assets/shaders/instanced.shd", "instanced");
    SetupDefaultProperties(instancedMeshMaterial);
    CreateInstancedMeshActor("assets/box.fbx", instancedMeshMaterial);

    PlaceLightsAndPlayer();
}

void SandboxGameLayer::Update(Duration deltaTime)
{
    float dt = deltaTime.GetSeconds();

    auto shader = ResourceManager::GetShader("assets/shaders/instanced.shd");
    shader->Use();
    m_LastDeltaSeconds = deltaTime;
}

void SandboxGameLayer::Render()
{
    static float v = 0.00f;
    static float accum = 0.0f;
    static int numFrames = 0;
    static float Max = 0.2f;

    glm::vec3 lightPosition{0.0f};

    {
        std::optional<Actor> directionalLight = m_Level->TryFindActor("directional_light");

        if (directionalLight.has_value())
        {
            lightPosition = directionalLight->GetTransform().Position;
        }
    }

    std::optional<Actor> characterActor = m_Level->TryFindActor("SkeletalMesh0");
    if (characterActor.has_value())
    {
        Transform transform = characterActor->GetTransform().GetAsTransform();
        transform.Scale = {1, 1, 1};
        Debug::DrawDebugBox(m_TestSkeletalMesh->GetBoundingBox(), transform);
    }

    // draw directional light gizmo
    for (int i = 0; i < 3; ++i)
    {
        glm::vec3 pos = glm::vec3{0.0f};
        glm::vec4 color = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
        pos[i] = 1;
        color[i] = 1;

        Line line{glm::zero<glm::vec3>(), pos};

        Debug::DrawDebugLine(line, Transform{lightPosition}, color);
    }

    Debug::DrawDebugLine(Line{lightPosition, lightPosition + 2.0f * glm::vec3{0, -1, 0}}, Transform{}, glm::vec4(0, 0, 1, 1));

    accum += m_LastDeltaSeconds.GetSeconds();

    if (accum > Max)
    {
        accum = 0;
        numFrames = (numFrames + 1) % 3;
        Max = std::fmod(Max, 0.4f) + 0.2f;
    }

    v += 10.0f * m_LastDeltaSeconds.GetSeconds();
    v = fmod(v, 360.0f);

    SpriteSheetData spriteSheetData{
        glm::uvec2(3, 1),
        glm::vec2(0, 0),
        glm::vec2(48, 48),
        ResourceManager::GetTexture2D("assets/fireworks.png")
    };

    Sprite2D spriteDefinition(glm::vec2(1000, 10), glm::vec2(200, 200),
        Renderer2D::BindTextureToDraw(ResourceManager::GetTexture2D("assets/fireworks.png")),
        spriteSheetData, glm::uvec2(numFrames, 0), RgbaColor(255, 255, 255));

    spriteDefinition.Transform.Rotation = v;

    Debug::DrawDebugRect(glm::vec2(1000, 10), glm::vec2(200, 200), Transform{}, glm::vec4(0.8f, 0.4f, 0.1f, 1.0f));
    ResourceManager::GetTexture2D("assets/fireworks.png")->SetFilteringType(FilteringType::Nearest);
    Renderer2D::DrawSprite(spriteDefinition);

    m_Skybox.Draw();
}

void SandboxGameLayer::OnImguiFrame()
{
    ImGui::ShowDemoWindow();

    {
        FpsCounter& counter = m_Level->FindActor("player").GetComponent<FpsCounter>();

        RenderStats stats = RenderCommand::GetRenderStats();

        ImGui::Begin("Stats");
        ImGui::Text("Fps: %i", static_cast<int>(round(1000.0 / counter.GetFrameTime())));
        ImGui::Text("Frame time: %.2f ms", counter.GetFrameTime());
        ImGui::Text("Drawcalls: %i", stats.NumDrawcalls);
        std::string text = FormatSize(IndexBuffer::s_IndexBufferMemoryAllocation);
        ImGui::Text("NumIndicesMemoryAllocated: %s", text.c_str());
        text = FormatSize(VertexBuffer::s_NumVertexBufferMemoryAllocated);
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

    for (auto& [name, actor] : *m_Level)
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

    ImGui::Begin("Test image");

    std::optional<Actor> instancedMeshActor = m_Level->TryFindActor("InstancedMesh");

    if (instancedMeshActor.has_value())
    {
        auto& instancedMesh = instancedMeshActor->GetComponent<InstancedMeshComponent>();

        auto material = instancedMesh.TargetInstancedMesh->GetMaterial();
        material->VisitForEachParam(*m_GuiDisplayVisitor);
    }

    ImGui::End();

    ImGui::ShowDemoWindow();
}

void SandboxGameLayer::OnImgizmoFrame()
{
    if (!m_SelectedActor.IsAlive())
    {
        return;
    }

    glm::mat4 transform = m_SelectedActor.GetTransform().GetWorldTransformMatrix();

    glm::mat4 view = Renderer::GetViewMatrix();
    glm::mat4 projection = Renderer::GetProjectionMatrix();

    if (ImGuizmo::Manipulate(&view[0][0], glm::value_ptr(projection),
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

bool SandboxGameLayer::OnKeyDown(KeyCode::Index keyCode)
{
    if (keyCode == KeyCode::P)
    {
        m_Game.lock()->SetMouseVisible(!m_Game.lock()->IsMouseVisible());
    }

    return true;
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
    if (auto game = m_Game.lock(); game->IsMouseVisible())
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
        std::string property_name = std::string{"Diffuse"} + std::to_string(i + 1);
        m_TestSkeletalMesh->MainMaterial->SetTextureProperty(property_name.c_str(), ResourceManager::GetTexture2D(m_TestSkeletalMesh->TextureNames[i]));
    }

    std::shared_ptr<Material> material = m_TestSkeletalMesh->MainMaterial;

    material->SetFloatProperty("Shininess", 32.0f);
}

void SandboxGameLayer::CreateSkeletalActors()
{
    for (int i = 0; i < 2; ++i)
    {
        Actor skeletalMeshActor = m_Level->CreateActor("SkeletalMesh" + std::to_string(i));
        skeletalMeshActor.AddComponent<SkeletalMeshComponent>(m_TestSkeletalMesh);
        skeletalMeshActor.GetComponent<TransformComponent>().Scale = glm::vec3{0.01f, 0.01f, 0.01f};
        skeletalMeshActor.GetComponent<TransformComponent>().Position = glm::vec3{0, -2 * i - 2, -i - 1};
    }
}

Actor SandboxGameLayer::CreateInstancedMeshActor(const std::string& filePath, const std::shared_ptr<Material>& material)
{
    Actor instanceMesh = m_Level->CreateActor("InstancedMesh");
    ResourceManager::GetStaticMesh(filePath)->LoadLod("assets/box_lod1.fbx", 1);
    ResourceManager::GetStaticMesh(filePath)->LoadLod("assets/box_lod2.fbx", 2);

    instanceMesh.AddComponent<InstancedMeshComponent>(ResourceManager::GetStaticMesh(filePath), material);

    material->SetTextureProperty("Diffuse1", ResourceManager::GetTexture2D("assets/T_Metal_Steel_D.TGA"));

    InstancedMeshComponent& instancedMesh = instanceMesh.GetComponent<InstancedMeshComponent>();

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 0; ++j)
        {
            Transform transform{glm::vec3{5.0f * i, 2.0f, 3.0f * j}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}};
            instancedMesh.AddInstance(transform);
        }
    }

    //instancedMesh.AddInstance(Transform(glm::vec3(25, 4, -40)));

    std::mt19937 m{};
    std::uniform_real_distribution<float> distribution{-10.0f, 10.0f};

    for (int i = 0; i < 0; ++i)
    {
        Transform transform{glm::vec3{distribution(m), distribution(m), distribution(m)}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}};
        Actor actor = m_Level->CreateActor("StaticMesh" + std::to_string(i));

        actor.AddComponent<StaticMeshComponent>(filePath);
        actor.GetTransform().Position = transform.Position;
    }

    return instanceMesh;
}

void SandboxGameLayer::PlaceLightsAndPlayer()
{
    Actor lightActor = m_Level->CreateActor("point_light");

    lightActor.AddComponent<PointLightComponent>();

    PointLightComponent& pointLight = lightActor.GetComponent<PointLightComponent>();

    pointLight.DirectionLength = 40.0f;
    pointLight.Color = glm::vec3{1, 0, 0};
    lightActor.GetTransform().Position = glm::vec3{4, 5, 0};

    Actor player = m_Level->CreateActor("player");
    player.AddComponent<PlayerController>(player);
    player.AddComponent<CameraComponent>();
    player.AddComponent<SpotLightComponent>();
    player.AddComponent<FpsCounter>();

    SpotLightComponent& playerSpotLight = player.GetComponent<SpotLightComponent>();
    playerSpotLight.CutOffAngle = 16.0f;
    playerSpotLight.OuterCutOffAngle = 20.0f;
    playerSpotLight.DirectionLength = 120;
    playerSpotLight.Direction = {0, 0, -1};
    playerSpotLight.Intensity = 0.0f;
    playerSpotLight.Color = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);

    Actor directionalLight = m_Level->CreateActor("directional_light");
    m_SelectedActor = directionalLight;
    directionalLight.AddComponent<DirectionalLightComponent>();

    DirectionalLightComponent& directional = directionalLight.GetComponent<DirectionalLightComponent>();
    directional.Direction = glm::normalize(glm::vec3{1, -1, 0});

    PlayerController& controller = player.GetComponent<PlayerController>();
    controller.BindForwardCallback(std::bind(&SandboxGameLayer::MoveForward, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindRightCallback(std::bind(&SandboxGameLayer::MoveRight, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindMouseMoveCallback(std::bind(&SandboxGameLayer::RotateCamera, this, std::placeholders::_1, std::placeholders::_2));

    m_Entity = m_Level->CreateEntity<StaticMeshEntity>("TestEntity");
    m_Entity->StaticMeshPath = "assets/box.fbx";

    m_Entity->SetLocalOrigin(glm::vec3(10, 0, -10));
}
