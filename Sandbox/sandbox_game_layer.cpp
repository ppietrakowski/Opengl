#include "sandbox_game_layer.h"

#include <functional>
#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

static void SetupDefaultProperties(const std::shared_ptr<Material>& material) {
    material->SetFloatProperty("reflection_factor", 0.05f);
    material->SetFloatProperty("shininess", 32.0f);
    material->transparent = true;
    material->cull_faces = false;
}

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
    static_mesh->main_material = default_material;
    default_material->transparent = true;

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

void SandboxGameLayer::Update(Duration delta_time) {
    float dt = delta_time.GetSeconds();

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
    last_delta_seconds_ = delta_time;

    level_.BroadcastUpdate(delta_time);
}

void SandboxGameLayer::Render(Duration delta_time) {
    const CameraComponent& camera_transform = level_.FindCameraComponent();
    Renderer::BeginScene(camera_transform.pos, camera_transform.rotation);

    glm::vec3 light_pos{0.0f};

    {
        Actor directional_light = level_.FindActor("directional_light");
        light_pos = directional_light.GetTransform().position;
    }

    current_used_shader_->Use();
    current_used_shader_->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    debug_shader_->Use();
    debug_shader_->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    Renderer::DrawDebugBox(test_skeletal_mesh_->GetBoundingBox(), Transform{glm::vec3{0, -2, -1}});

    // draw directional light gizmo
    for (int i = 0; i < 3; ++i) {
        glm::vec3 pos = glm::vec3{0.0f};
        glm::vec4 color = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
        pos[i] = 1;
        color[i] = 1;

        Line line{light_pos, light_pos + pos};

        Renderer::DrawDebugLine(line, Transform{}, color);
    }

    Renderer::DrawDebugLine(Line{light_pos, light_pos + 2.0f * glm::vec3{0, -1, 0}}, Transform{}, glm::vec4(0, 0, 1, 1));

    Renderer2D::DrawRect(glm::vec2{1000, 10}, glm::vec2{1200, 50}, Transform2D{}, RgbaColor(255, 255, 255),
        Renderer2D::BindTextureToDraw(ResourceManager::GetTexture2D("assets/test_hp.png")));

    level_.BroadcastRender(delta_time);
    skybox_->Draw();
    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event) {

    if (event.type == EventType::kKeyPressed && event.key.key == GLFW_KEY_P) {
        game_->SetMouseVisible(!game_->IsMouseVisible());
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame() {
    static int last_framerate = 0;
    static int num_frame{0};

    if (num_frame == 2) {
        last_framerate = (last_framerate + static_cast<int>(1000 / last_delta_seconds_.GetMilliseconds())) / 2;
    } else {
        num_frame++;
    }

    RenderStats stats = RenderCommand::GetRenderStats();

    ImGui::Begin("Stats");
    ImGui::Text("Fps: %i", last_framerate);
    ImGui::Text("Frame time: %.2f ms", last_delta_seconds_.GetMilliseconds());
    ImGui::Text("Drawcalls: %i", stats.num_drawcalls);
    ImGui::Text("NumIndicesMemoryAllocated: %s", FormatSize(stats.index_bufer_memory_allocation));
    ImGui::Text("NumVerticesMemoryAllocated: %s", FormatSize(stats.vertex_buffer_memory_allocation));
    ImGui::Text("NumBytesUniformBuffer: %s", FormatSize((int)UniformBuffer::num_bytes_allocated));
    ImGui::Text("NumTextureMemoryUsage: %s", FormatSize((int)Texture2D::num_texture_vram_used));
    ImGui::End();

    Actor actor_to_remove{};
    std::string removed_actor_name;

    ImGui::Begin("Hierarchy");

    for (auto& [name, actor] : level_) {
        ImGui::Button(name.c_str());
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
    Actor actor{};

    if (!level_.TryFindActor("point_light", actor)) {
        return;
    }

    glm::mat4 transform = actor.GetTransform().GetWorldTransformMatrix();

    if (ImGuizmo::Manipulate(glm::value_ptr(Renderer::view_), glm::value_ptr(Renderer::projection_),
        ImGuizmo::OPERATION::UNIVERSAL, ImGuizmo::LOCAL, &transform[0][0]
    )) {

        glm::vec3 pos, rot, scale;

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), &pos[0], &rot[0], &scale[0]);

        actor.GetTransform().SetEulerAngles(rot);
        actor.GetTransform().position = pos;
        actor.GetTransform().scale = scale;
    }
}

void SandboxGameLayer::MoveForward(Actor& player, float axis_value) {
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 forward = axis_value * transform.GetForwardVector() * last_delta_seconds_.GetSeconds() * move_speed_;
    transform.Translate(forward);
}

void SandboxGameLayer::MoveRight(Actor& player, float axis_value) {
    TransformComponent& transform = player.GetComponent<TransformComponent>();
    glm::vec3 right = axis_value * transform.GetRightVector() * last_delta_seconds_.GetSeconds() * move_speed_;
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
    test_skeletal_mesh_->main_material = ResourceManager::CreateMaterial("assets/shaders/skeletal_default.shd", "skeletal1");

    for (int i = 0; i < test_skeletal_mesh_->texture_names.size(); ++i) {
        std::string property_name = std::string{"diffuse"} + std::to_string(i + 1);
        test_skeletal_mesh_->main_material->SetTextureProperty(property_name.c_str(), ResourceManager::GetTexture2D(test_skeletal_mesh_->texture_names[i]));
    }

    std::shared_ptr<Material> material = test_skeletal_mesh_->main_material;

    material->cull_faces = false;
    material->SetFloatProperty("shininess", 32.0f);
}

void SandboxGameLayer::CreateSkeletalActors() {
    for (std::int32_t i = 0; i < 2; ++i) {
        Actor skeletal_mesh_actor = level_.CreateActor("SkeletalMesh" + std::to_string(i));
        skeletal_mesh_actor.AddComponent<SkeletalMeshComponent>(test_skeletal_mesh_);
        skeletal_mesh_actor.GetComponent<TransformComponent>().scale = glm::vec3{0.01f, 0.01f, 0.01f};
        skeletal_mesh_actor.GetComponent<TransformComponent>().position = glm::vec3{0, -2 * i - 2, -i - 1};
    }
}

Actor SandboxGameLayer::CreateInstancedMeshActor(const std::string& file_path, const std::shared_ptr<Material>& material) {
    Actor instance_mesh = level_.CreateActor("InstancedMesh");
    instance_mesh.AddComponent<InstancedMeshComponent>(ResourceManager::GetStaticMesh(file_path), material);

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

    point_light.direction_length = 40.0f;
    point_light.direction = glm::vec3{0, -1, 0};
    point_light.color = glm::vec3{1, 0, 0};
    point_light.cut_off_angle = 45.0f;
    light_actor.GetTransform().position = glm::vec3{4, 5, 0};

    Actor player = level_.CreateActor("player");
    player.AddComponent<PlayerController>(player);
    player.AddComponent<CameraComponent>();

    Actor directional_light = level_.CreateActor("directional_light");
    directional_light.AddComponent<DirectionalLightComponent>();

    DirectionalLightComponent& directional = directional_light.GetComponent<DirectionalLightComponent>();
    directional.direction = {0, -1, 0};

    PlayerController& controller = player.GetComponent<PlayerController>();
    controller.BindForwardCallback(std::bind(&SandboxGameLayer::MoveForward, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindRightCallback(std::bind(&SandboxGameLayer::MoveRight, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindMouseMoveCallback(std::bind(&SandboxGameLayer::RotateCamera, this, std::placeholders::_1, std::placeholders::_2));

}
