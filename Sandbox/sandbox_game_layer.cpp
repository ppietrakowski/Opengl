#include "sandbox_game_layer.h"

#include "renderer.h"
#include "Imgui/imgui.h"
#include "error_macros.h"
#include "logging.h"

#include <glm/gtx/matrix_decompose.hpp>

SandboxGameLayer::SandboxGameLayer() :
    camera_rotation_{glm::vec3{0, 0, 0}},
    camera_position_{0.0f, 0.0f, 0.0f} {
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
    default_material_->SetFloatProperty("shininess", 87.2f);
    debug_material_->use_wireframe = true;
    current_material_ = default_material_;
    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    static_mesh_ = ResourceManager::GetStaticMesh("postac.obj");
    static_mesh_->main_material = default_material_;

    uint32_t i = 0;

    std::shared_ptr<Material> material = test_skeletal_mesh_->main_material;

    for (const auto& path : test_skeletal_mesh_->textures) {
        std::string name = "diffuse" + std::to_string(i + 1);
        material->SetTextureProperty(name.c_str(), ResourceManager::GetTexture2D(path));
        ++i;
    }

    camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    static_mesh_position_ = {2, 0, -10};
    std::vector<std::string> animations = std::move(test_skeletal_mesh_->GetAnimationNames());
    RenderCommand::SetClearColor(RgbaColor{50, 30, 170});

    for (int i = 0; i < 2; ++i) {
        skeletal_mesh_actor_ = level_.CreateActor("SkeletalMesh" + std::to_string(i));
        skeletal_mesh_actor_.AddComponent<SkeletalMeshComponent>(test_skeletal_mesh_);
        skeletal_mesh_actor_.GetComponent<TransformComponent>().scale = glm::vec3{0.01f, 0.01f, 0.01f};
        skeletal_mesh_actor_.GetComponent<TransformComponent>().position = glm::vec3{0, -2 * i - 2, -i - 1};
    }

    Actor static_mesh_actor = level_.CreateActor("StaticMeshActor");
    static_mesh_actor.AddComponent<StaticMeshComponent>(static_mesh_);
    static_mesh_actor.GetComponent<TransformComponent>().SetLocalEulerAngles(glm::vec3{0, 90, 0});

    bbox_min_ = test_skeletal_mesh_->GetBboxMin();
    bbox_max_ = test_skeletal_mesh_->GetBboxMax();
    instanced_mesh_ = std::make_shared<InstancedMesh>(static_mesh_);
}

void SandboxGameLayer::Update(Duration delta_time) {
    float dt = delta_time.GetAsSeconds();

    if (Input::IsKeyPressed(Keys::kW)) {
        glm::vec3 world_forward = glm::vec3{0, 0, -1};
        glm::vec3 forward = camera_rotation_ * world_forward * dt * move_speed_;
        camera_position_ += forward;
    } else if (Input::IsKeyPressed(Keys::kS)) {
        glm::vec3 world_backward = glm::vec3{0, 0, 1};
        glm::vec3 backward = camera_rotation_ * world_backward * dt * move_speed_;
        camera_position_ += backward;
    }

    if (Input::IsKeyPressed(Keys::kE)) {
        camera_yaw_ -= yaw_rotation_rate_ * dt;
        camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    } else if (Input::IsKeyPressed(Keys::kQ)) {
        camera_yaw_ += yaw_rotation_rate_ * dt;
        camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
    }

    if (Input::IsKeyPressed(Keys::kY)) {
        glm::vec3 world_up = glm::vec3{0, 1, 0};
        camera_position_ += ascend_speed_ * world_up * dt;
    } else if (Input::IsKeyPressed(Keys::kH)) {
        glm::vec3 world_down = glm::vec3{0, -1, 0};
        camera_position_ += ascend_speed_ * world_down * dt;
    }
    last_delta_seconds_ = delta_time;

    level_.BroadcastUpdate(delta_time);
}

void SandboxGameLayer::Render(Duration delta_time) {
    Renderer::BeginScene(glm::inverse(glm::translate(camera_position_) * glm::mat4_cast(camera_rotation_)), camera_position_, camera_rotation_);
    current_used_shader_->Use();
    current_used_shader_->SetUniform("u_material.diffuse", glm::vec3{0.34615f, 0.3143f, 0.0903f});

    for (int i = 0; i < 100; ++i) {
        Transform transform{glm::vec3{5.0f * i, 2.0f, 1.0f}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}};
        instanced_mesh_->QueueDraw(transform, 0);
        Renderer::DrawDebugBox(instanced_mesh_->GetMesh().GetBBoxMin(), instanced_mesh_->GetMesh().GetBBoxMax(), transform);
    }

    instanced_mesh_->Draw(glm::identity<glm::mat4>(), *static_mesh_->main_material);
    debug_shader_->Use();
    debug_shader_->SetUniform("u_material.diffuse", glm::vec3{1, 0, 0});
    Renderer::DrawDebugBox(static_mesh_->GetBBoxMin(), static_mesh_->GetBBoxMax(), Transform{static_mesh_position_, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 0, 1});
    Renderer::DrawDebugBox(static_mesh_->GetBBoxMin(), static_mesh_->GetBBoxMax(), Transform{static_mesh_position_ + glm::vec3{10, 0, 0}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 0, 1});
    Renderer::DrawDebugBox(test_skeletal_mesh_->GetBboxMin(), test_skeletal_mesh_->GetBboxMax(), Transform{glm::vec3{0, -2, -1}});

    Renderer::DrawDebugBox(bbox_min_, bbox_max_, Transform{glm::vec3{10, 2, 10}, glm::quat{glm::vec3{0, 0, 0}}, glm::vec3{1, 1, 1}}, glm::vec4{1, 0, 1, 1});
    level_.BroadcastRender(delta_time);
    Renderer::FlushDrawDebug(*debug_material_);
    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event) {
    if (event.type == EventType::kMouseMoved) {
        glm::vec2 delta = event.mouse_move.mouse_position - event.mouse_move.last_mouse_position;
        float dt = last_delta_seconds_.GetAsSeconds();

        camera_yaw_ -= yaw_rotation_rate_ * delta.x * dt;
        camera_pitch_ -= pitch_rotation_rate_ * delta.y * dt;

        if (camera_pitch_ < -89) {
            camera_pitch_ = -89;
        }
        if (camera_pitch_ > 89) {
            camera_pitch_ = 89;
        }

        camera_rotation_ = glm::quat{glm::radians(glm::vec3{camera_pitch_, camera_yaw_, 0.0f})};
        return true;
    }

    if (event.type == EventType::kMouseButtonPressed) {
        DO_ONCE([this]() {
            Actor actor = level_.FindActor("StaticMeshActor");
            actor.DestroyActor();
        }
        );
    }

    if (event.type == EventType::kKeyPressed && event.key_event.code == Keys::kP) {
        if (current_material_.get() == debug_material_.get()) {
            current_material_ = default_material_;
        } else {
            current_material_ = debug_material_;
        }
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame() {
    static std::int32_t last_framerate = 0;
    static std::int32_t num_frame{0};

    if (num_frame == 2) {
        last_framerate = (last_framerate + static_cast<std::int32_t>(1000 / last_delta_seconds_.GetAsMilliseconds())) / 2;
    } else {
        num_frame++;
    }

    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &static_mesh_position_[0], -10, 10);

    RenderStats stats = RenderCommand::GetRenderStats();

    ImGui::Text("Fps: %i", last_framerate);
    ImGui::Text("Frame time: %.2f ms", last_delta_seconds_.GetAsMilliseconds());
    ImGui::Text("Drawcalls: %u", stats.num_drawcalls);
    ImGui::End();
}
