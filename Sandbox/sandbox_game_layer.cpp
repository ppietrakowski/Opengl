#include "sandbox_game_layer.h"

#include <functional>
#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

static void SetupDefaultProperties(const std::shared_ptr<Material>& material) {
    material->SetFloatProperty("reflection_factor", 0.1f);
    material->SetFloatProperty("shininess", 32.0f);
    material->bTransparent = true;
    material->bCullFaces = false;
}

struct FpsCounter {
    float total_frametime = 1.0f;
    milliseconds_float_t frame_time = milliseconds_float_t::zero();
    FpsCounter() {
    }
     
    double GetFrameTime() const {
        return frame_time.count();
    }

    void Tick(Duration deltaSeconds) {
        total_frametime += deltaSeconds.GetSeconds();

        if (total_frametime >= 1.0f) {
            total_frametime = 0.0f;
            frame_time = deltaSeconds.GetChronoNanoSeconds();
        }
    }
};

DECLARE_COMPONENT_TICKABLE(FpsCounter);

SandboxGameLayer::SandboxGameLayer(Game* game) :
    camera_rotation_{glm::vec3{0, 0, 0}},
    camera_position_{0.0f, 0.0f, 0.0f},
    game_(game) {
    default_shader_ = ResourceManager::GetShader("assets/shaders/default.shd");
    debug_shader_ = ResourceManager::GetShader("assets/shaders/unshaded.shd");

    ResourceManager::CreateMaterial("assets/shaders/default.shd", "default");
    InitializeSkeletalMesh();

    current_used_shader_ = default_shader_;
    glm::vec3 white{1.0f, 1.0f, 1.0f};

    Renderer::InitializeDebugDraw(debug_shader_);
    Renderer2D::SetDrawShader(ResourceManager::GetShader("assets/shaders/sprite_2d.shd"));

    auto default_material = ResourceManager::CreateMaterial("assets/shaders/default.shd", "postac_material");
    default_material->SetFloatProperty("shininess", 32.0f);

    std::shared_ptr<StaticMesh> static_mesh = ResourceManager::GetStaticMesh("assets/cube.obj");
    static_mesh->MainMaterial = default_material;
    default_material->bTransparent = true;

    camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};

    CreateSkeletalActors();

    std::shared_ptr<Material> instanced_mesh_material = ResourceManager::CreateMaterial("assets/shaders/instanced.shd", "instanced");
    SetupDefaultProperties(instanced_mesh_material);
    instanced_mesh_ = std::make_shared<InstancedMesh>(static_mesh, instanced_mesh_material);

    CreateInstancedMeshActor("assets/cube.obj", instanced_mesh_material);

    PlaceLightsAndPlayer();

    std::array<std::string, 6> texture_paths = {
        "assets/skybox/right.jpg",
        "assets/skybox/left.jpg",
        "assets/skybox/top.jpg",
        "assets/skybox/bottom.jpg",
        "assets/skybox/front.jpg",
        "assets/skybox/back.jpg"
    };

    skybox_ = std::make_unique<Skybox>(std::make_shared<CubeMap>(texture_paths), ResourceManager::GetShader("assets/shaders/skybox.shd"));
}

void SandboxGameLayer::Update(Duration deltaTime) {
    float dt = deltaTime.GetSeconds();

    auto shader = ResourceManager::GetShader("assets/shaders/instanced.shd");
    shader->Use();
    if (Input::IsKeyPressed(GLFW_KEY_E)) {
        camera_yaw_ -= yaw_rotation_rate_ * dt;
        camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    } else if (Input::IsKeyPressed(GLFW_KEY_Q)) {
        camera_yaw_ += yaw_rotation_rate_ * dt;
        camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    }

    if (Input::IsKeyPressed(GLFW_KEY_Y)) {
        glm::vec3 world_up = glm::vec3{0, 1, 0};
        camera_position_ += ascend_speed_ * world_up * dt;
    } else if (Input::IsKeyPressed(GLFW_KEY_H)) {
        glm::vec3 world_down = glm::vec3{0, -1, 0};
        camera_position_ += ascend_speed_ * world_down * dt;
    }
    last_delta_seconds_ = deltaTime;

    level_.BroadcastUpdate(deltaTime);
}

void SandboxGameLayer::Render() {
    Renderer::BeginScene(level_.CameraPosition, level_.CameraRotation);

    glm::vec3 light_pos{0.0f};

    {
        Actor directional_light = level_.FindActor("directional_light");
        light_pos = directional_light.GetTransform().Position;
    }

    current_used_shader_->Use();
    current_used_shader_->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    debug_shader_->Use();
    debug_shader_->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    Actor character_actor;

    if (level_.TryFindActor("SkeletalMesh0", character_actor)) {
        Renderer::DrawDebugBox(test_skeletal_mesh_->GetBoundingBox(), character_actor.GetTransform().GetAsTransform());
    }

    // draw directional light gizmo
    for (int i = 0; i < 3; ++i) {
        glm::vec3 pos = glm::vec3{0.0f};
        glm::vec4 Color = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
        pos[i] = 1;
        Color[i] = 1;

        Line line{light_pos, light_pos + pos};

        Renderer::DrawDebugLine(line, Transform{}, Color);
    }

    Renderer::DrawDebugLine(Line{light_pos, light_pos + 2.0f * glm::vec3{0, -1, 0}}, Transform{}, glm::vec4(0, 0, 1, 1));

    level_.BroadcastRender();
    skybox_->Draw();
    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event) {
    if (event.Type == EventType::KeyPressed && event.Key.Key == GLFW_KEY_P) {
        game_->SetMouseVisible(!game_->IsMouseVisible());
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame() {
    static int num_frame{0};
    {
        FpsCounter& counter = level_.FindActor("player").GetComponent<FpsCounter>();

        RenderStats stats = RenderCommand::GetRenderStats();

        ImGui::Begin("Stats");
        ImGui::Text("Fps: %i", static_cast<int>(round(1000.0 / counter.GetFrameTime())));
        ImGui::Text("Frame time: %.2f ms", counter.GetFrameTime());
        ImGui::Text("Drawcalls: %i", stats.NumDrawcalls);
        ImGui::Text("NumIndicesMemoryAllocated: %s", FormatSize(stats.IndexBufferMemoryAllocation));
        ImGui::Text("NumVerticesMemoryAllocated: %s", FormatSize(stats.VertexBufferMemoryAllocation));
        ImGui::Text("NumBytesUniformBuffer: %s", FormatSize(UniformBuffer::NumBytesAllocated));
        ImGui::Text("NumTextureMemoryUsage: %s", FormatSize(Texture2D::s_NumTextureVramUsed));
        ImGui::End();
    }

    Actor actor_to_remove{};
    std::string removed_actor_name;

    ImGui::Begin("Hierarchy");

    for (auto& [name, actor] : level_) {
        if (ImGui::Button(name.c_str())) {
            selected_actor = actor;
            break;
        }

        ImGui::SameLine();

        ImGui::PushID(name.c_str());
        if (ImGui::Button("X")) {
            actor_to_remove = actor;
            removed_actor_name = name;
        }
        ImGui::PopID();

        ImGui::NextColumn();
    }

    ImGui::End();

    if (removed_actor_name != "player" && !removed_actor_name.empty()) {
        actor_to_remove.DestroyActor();
    }
}

void SandboxGameLayer::OnImgizmoFrame() {
    if (!selected_actor.IsAlive()) {
        return;
    }

    glm::mat4 transform = selected_actor.GetTransform().GetWorldTransformMatrix();

    if (ImGuizmo::Manipulate(glm::value_ptr(Renderer::s_View), glm::value_ptr(Renderer::s_Projection),
        ImGuizmo::OPERATION::UNIVERSAL, ImGuizmo::WORLD, glm::value_ptr(transform)
    )) {

        glm::vec3 pos, rot, scale;

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), &pos[0], &rot[0], &scale[0]);

        TransformComponent& transform = selected_actor.GetTransform();
        transform.SetEulerAngles(rot);
        transform.Position = pos;
        transform.Scale = scale;
    }
}

void SandboxGameLayer::MoveForward(Actor& player, float axis_value) {
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 forward = axis_value * transform.GetForwardVector() * static_cast<float>(last_delta_seconds_.GetSeconds()) * move_speed_;
    transform.Translate(forward);
}

void SandboxGameLayer::MoveRight(Actor& player, float axis_value) {
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 right = axis_value * transform.GetRightVector() * static_cast<float>(last_delta_seconds_.GetSeconds()) * move_speed_;
    transform.Translate(right);
}

void SandboxGameLayer::RotateCamera(Actor& player, glm::vec2 mouse_move_delta) {
    if (game_->IsMouseVisible()) {
        return;
    }

    float dt = last_delta_seconds_.GetSeconds();

    TransformComponent& transform = player.GetComponent<TransformComponent>();

    camera_yaw_ -= yaw_rotation_rate_ * mouse_move_delta.x * dt;
    camera_pitch_ -= pitch_rotation_rate_ * mouse_move_delta.y * dt;

    if (camera_pitch_ < -89) {
        camera_pitch_ = -89;
    }
    if (camera_pitch_ > 89) {
        camera_pitch_ = 89;
    }

    transform.SetEulerAngles(camera_pitch_, camera_yaw_, 0.0f);
}

void SandboxGameLayer::InitializeSkeletalMesh() {
    auto skeletal_shader = ResourceManager::GetShader("assets/shaders/skeletal_default.shd");
    test_skeletal_mesh_ = ResourceManager::GetSkeletalMesh("assets/test_character.fbx");
    test_skeletal_mesh_->MainMaterial = ResourceManager::CreateMaterial("assets/shaders/skeletal_default.shd", "skeletal1");

    for (int i = 0; i < test_skeletal_mesh_->TextureNames.size(); ++i) {
        std::string property_name = std::string{"diffuse"} + std::to_string(i + 1);
        test_skeletal_mesh_->MainMaterial->SetTextureProperty(property_name.c_str(), ResourceManager::GetTexture2D(test_skeletal_mesh_->TextureNames[i]));
    }

    std::shared_ptr<Material> material = test_skeletal_mesh_->MainMaterial;

    material->bCullFaces = false;
    material->SetFloatProperty("shininess", 32.0f);
}

void SandboxGameLayer::CreateSkeletalActors() {
    for (std::int32_t i = 0; i < 2; ++i) {
        Actor skeletal_mesh_actor = level_.CreateActor("SkeletalMesh" + std::to_string(i));
        skeletal_mesh_actor.AddComponent<SkeletalMeshComponent>(test_skeletal_mesh_);
        skeletal_mesh_actor.GetComponent<TransformComponent>().Scale = glm::vec3{0.01f, 0.01f, 0.01f};
        skeletal_mesh_actor.GetComponent<TransformComponent>().Position = glm::vec3{0, -2 * i - 2, -i - 1};
    }
}

Actor SandboxGameLayer::CreateInstancedMeshActor(const std::string& filePath, const std::shared_ptr<Material>& material) {
    Actor instance_mesh = level_.CreateActor("InstancedMesh");
    instance_mesh.AddComponent<InstancedMeshComponent>(ResourceManager::GetStaticMesh(filePath), material);

    material->SetTextureProperty("diffuse1", ResourceManager::GetTexture2D("assets/T_Metal_Steel_D.TGA"));

    InstancedMeshComponent& instanced_mesh = instance_mesh.GetComponent<InstancedMeshComponent>();

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 400; ++j) {
            Transform transform{glm::vec3{5.0f * i, 2.0f, 3.0f * j}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}};
            instanced_mesh.AddInstance(transform);
        }
    }

    return instance_mesh;
}

void SandboxGameLayer::PlaceLightsAndPlayer() {
    Actor light_actor = level_.CreateActor("point_light");

    light_actor.AddComponent<SpotLightComponent>();

    SpotLightComponent& point_light = light_actor.GetComponent<SpotLightComponent>();

    point_light.DirectionLength = 40.0f;
    point_light.Direction = glm::vec3{0, -1, 0};
    point_light.Color = glm::vec3{1, 0, 0};
    point_light.CutOffAngle = 45.0f;
    light_actor.GetTransform().Position = glm::vec3{4, 5, 0};

    Actor player = level_.CreateActor("player");
    player.AddComponent<PlayerController>(player);
    player.AddComponent<CameraComponent>();
    player.AddComponent<FpsCounter>();

    Actor directional_light = level_.CreateActor("directional_light");
    selected_actor = directional_light;
    directional_light.AddComponent<DirectionalLightComponent>();

    DirectionalLightComponent& directional = directional_light.GetComponent<DirectionalLightComponent>();
    directional.Direction = {0, -1, 0};

    PlayerController& controller = player.GetComponent<PlayerController>();
    controller.BindForwardCallback(std::bind(&SandboxGameLayer::MoveForward, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindRightCallback(std::bind(&SandboxGameLayer::MoveRight, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindMouseMoveCallback(std::bind(&SandboxGameLayer::RotateCamera, this, std::placeholders::_1, std::placeholders::_2));

}
