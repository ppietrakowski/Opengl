#include "sandbox_game_layer.h"

#include <functional>
#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

struct TestComponent {
    float total_time{0.0f};

    void Tick(float delta_seconds) {
        total_time += delta_seconds;

        if (total_time >= 1.0f) {
            puts("OOKK");
            total_time = 0.0f;
        }
    }
};

DECLARE_COMPONENT_TICKABLE(TestComponent);

SandboxGameLayer::SandboxGameLayer(Game* game) :
    camera_rotation_{glm::vec3{0, 0, 0}},
    camera_position_{0.0f, 0.0f, 0.0f},
    game_(game) {
    default_shader_ = ResourceManager::GetShader("shaders/default.shd");
    debug_shader_ = ResourceManager::GetShader("shaders/unshaded.shd");
    auto skeletal_shader = ResourceManager::GetShader("shaders/skeletal_default.shd");

    ResourceManager::CreateMaterial("shaders/default.shd", "default");

    test_skeletal_mesh_ = ResourceManager::GetSkeletalMesh("untitled.fbx");
    test_skeletal_mesh_->main_material = ResourceManager::CreateMaterial("shaders/skeletal_default.shd", "skeletal1");
    current_used_shader_ = default_shader_;
    current_used_shader_->Use();
    current_used_shader_->SetUniform("u_light_color", glm::vec3{1, 1, 1});
    current_used_shader_->SetUniform("u_light_pos", glm::vec3{-1, 0, -5});
    default_shader_->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    current_used_shader_->SetUniform("u_material.ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    current_used_shader_->SetUniform("u_material.specular", glm::vec3{0.797357, 0.723991, 0.208006});
    current_used_shader_->SetUniform("u_material.shininess", 87.2f);

    debug_shader_->Use();
    debug_shader_->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    current_used_shader_->Use();

    default_shader_->SetUniform("u_projection_view", glm::identity<glm::mat4>());
    default_shader_->SetUniform("u_transform", glm::identity<glm::mat4>());
    glm::vec3 white{1.0f, 1.0f, 1.0f};

    default_shader_->SetUniform("u_material.Tint", white);

    debug_material_ = ResourceManager::CreateMaterial("shaders/unshaded.shd", "wireframe");
    default_material_ = ResourceManager::CreateMaterial("shaders/default.shd", "postac_material");

    default_material_->SetVector3Property("diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});
    default_material_->SetVector3Property("ambient", glm::vec3{0.01f, 0.01f, 0.01f});
    default_material_->SetVector3Property("specular", glm::vec3{0.797357, 0.723991, 0.208006});
    default_material_->SetFloatProperty("shininess", 32.0f);
    debug_material_->use_wireframe = true;
    current_material_ = default_material_;
    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    static_mesh_ = ResourceManager::GetStaticMesh("cube.obj");
    static_mesh_->main_material = default_material_;

    std::shared_ptr<Material> material = test_skeletal_mesh_->main_material;
    std::uint32_t texture_unit = 0;

    camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    static_mesh_position_ = {2, 0, -10};
    std::vector<std::string> animations = std::move(test_skeletal_mesh_->GetAnimationNames());

    for (std::int32_t i = 0; i < 2; ++i) {
        skeletal_mesh_actor_ = level_.CreateActor("SkeletalMesh" + std::to_string(i));
        skeletal_mesh_actor_.AddComponent<SkeletalMeshComponent>(test_skeletal_mesh_);
        skeletal_mesh_actor_.GetComponent<TransformComponent>().scale = glm::vec3{0.01f, 0.01f, 0.01f};
        skeletal_mesh_actor_.GetComponent<TransformComponent>().position = glm::vec3{0, -2 * i - 2, -i - 1};
    }

    Actor static_mesh_actor = level_.CreateActor("StaticMeshActor");
    static_mesh_actor.AddComponent<StaticMeshComponent>("cube.obj");
    static_mesh_actor.GetComponent<TransformComponent>().SetEulerAngles(glm::vec3{0, 90, 0});

    bbox_min_ = test_skeletal_mesh_->GetBboxMin();
    bbox_max_ = test_skeletal_mesh_->GetBboxMax();

    auto shader = ResourceManager::GetShader("shaders/instanced.shd");
    shader->Use();
    auto mat = ResourceManager::CreateMaterial("shaders/instanced.shd", "instanced");

    mat->SetVector3Property("diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});
    mat->SetVector3Property("ambient", 4.0f * glm::vec3{0.0034615f, 0.003143f, 0.000903f});
    mat->SetVector3Property("specular", glm::vec3{0.797357, 0.723991, 0.208006});
    mat->SetFloatProperty("shininess", 32);
    instanced_mesh_ = std::make_shared<InstancedMesh>(static_mesh_, ResourceManager::GetMaterial("instanced"));

    Actor instance_mesh = level_.CreateActor("InstancedMesh");

    instance_mesh.AddComponent<InstancedMeshComponent>(static_mesh_, ResourceManager::GetMaterial("instanced"));

    InstancedMeshComponent& instanced_mesh = instance_mesh.GetComponent<InstancedMeshComponent>();

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 400; ++j) {
            Transform transform{glm::vec3{5.0f * i, 2.0f, 3.0f * j}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}};
            instanced_mesh.AddInstance(transform);
        }
    }

    Actor light_actor = level_.CreateActor("point_light");

    light_actor.AddComponent<SpotLightComponent>();

    SpotLightComponent& point_light = light_actor.GetComponent<SpotLightComponent>();

    point_light.direction_length = 40.0f;
    point_light.direction = glm::vec3{0, -1, 0};
    point_light.color = glm::vec3{1, 0, 0};
    point_light.cut_off_angle = 45.0f;
    light_actor.GetTransform().position = light_pos_ws_ - glm::vec3{0, -5, 0};

    player_ = level_.CreateActor("Player");
    player_.AddComponent<PlayerController>(player_);
    player_.AddComponent<TestComponent>();

    //player_.AddComponent<SpotLightComponent>();
    //SpotLightComponent& player_spot_light = player_.GetComponent<SpotLightComponent>();
    //
    //player_spot_light.cut_off_angle = 28.0f;
    //player_spot_light.color = glm::vec3{0, 0, 1.0f};
    //player_spot_light.direction = {0, 0, -1};
    //player_spot_light.direction_length = 70.0f;
    //player_spot_light.intensity = 10;

    {
        Actor directional_light = level_.CreateActor("directional_light");
        directional_light.AddComponent<DirectionalLightComponent>();

        auto& directional = directional_light.GetComponent<DirectionalLightComponent>();
        directional.direction = {0, -1, 0};
    }

    PlayerController& controller = player_.GetComponent<PlayerController>();
    controller.BindForwardCallback(std::bind(&SandboxGameLayer::MoveForward, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindRightCallback(std::bind(&SandboxGameLayer::MoveRight, this, std::placeholders::_1, std::placeholders::_2));
    controller.BindMouseMoveCallback(std::bind(&SandboxGameLayer::RotateCamera, this, std::placeholders::_1, std::placeholders::_2));
}

void SandboxGameLayer::Update(Duration delta_time) {
    float dt = delta_time.GetSeconds();

    auto shader = ResourceManager::GetShader("shaders/instanced.shd");
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
    Transform camera_transform = player_.GetComponent<TransformComponent>().GetAsTransform();
    Renderer::BeginScene(camera_transform.position, camera_transform.rotation);

    current_used_shader_->Use();
    current_used_shader_->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    debug_shader_->Use();
    debug_shader_->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    Renderer::DrawDebugBox(static_mesh_->GetBoundingBox(), Transform{static_mesh_position_, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 0, 1});
    Renderer::DrawDebugBox(static_mesh_->GetBoundingBox(), Transform{static_mesh_position_ + glm::vec3{10, 0, 0}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 0, 1});
    Renderer::DrawDebugBox(test_skeletal_mesh_->GetBoundingBox(), Transform{glm::vec3{0, -2, -1}});

    level_.BroadcastRender(delta_time);

    for (int i = 0; i < 3; ++i) {
        glm::vec3 pos = glm::vec3{0.0f};
        glm::vec4 color = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
        pos[i] = 1;
        color[i] = 1;

        Line line{light_pos_ws_, light_pos_ws_ + pos};

        Renderer::DrawDebugLine(line, Transform{}, color);
    }

    Renderer::DrawDebugLine(Line{light_pos_ws_, light_pos_ws_ + 2.0f * glm::vec3{0, -1, 0}}, Transform{}, glm::vec4(0, 0, 1, 1));
    Renderer::FlushDrawDebug(*debug_material_);

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

    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &static_mesh_position_[0], -10, 10);

    RenderStats stats = RenderCommand::GetRenderStats();

    ImGui::Text("Fps: %i", last_framerate);
    ImGui::Text("Frame time: %.2f ms", last_delta_seconds_.GetMilliseconds());
    ImGui::Text("Drawcalls: %i", stats.num_drawcalls);
    ImGui::Text("NumIndicesMemoryAllocated: %s", FormatSize(stats.index_bufer_memory_allocation));
    ImGui::Text("NumVerticesMemoryAllocated: %s", FormatSize(stats.vertex_buffer_memory_allocation));
    ImGui::Text("NumBytesUniformBuffer: %s", FormatSize((int)UniformBuffer::num_bytes_allocated));
    ImGui::Text("NumTextureMemoryUsage: %s", FormatSize((int)Texture2D::num_texture_vram_used));
    ImGui::End();

    Actor actor_to_remove = player_;

    ImGui::Begin("Hierarchy");
    for (auto& [name, actor] : level_) {
        ImGui::Button(name.c_str());
        ImGui::SameLine();

        ImGui::PushID(name.c_str());
        if (ImGui::Button("X")) {
            actor_to_remove = actor;
        }
        ImGui::PopID();

        ImGui::NextColumn();
    }

    ImGui::End();

    if (actor_to_remove != player_) {
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