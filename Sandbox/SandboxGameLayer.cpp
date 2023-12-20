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
    camera_rotation_{ glm::vec3{0, 0, 0} },
    camera_position_{ 0.0f, 0.0f, 0.0f },
    skeletal_mesh_{ "untitled.fbx", std::make_shared<Material>(Shader::LoadShader("skeleton.vert", "textured.frag")) } {
    shader_ = Shader::LoadShader("shaders/default.vert", "shaders/default.frag");
    unshaded_ = Shader::LoadShader("shaders/default.vert", "shaders/Unshaded.frag");

    current_used_ = shader_;
    current_used_->Use();
    current_used_->SetUniformVec3("u_LightColor", glm::vec3{ 1, 1, 1 });
    current_used_->SetUniformVec3("u_LightPos", glm::vec3{ -1, 0, -5 });
    shader_->SetUniformVec3("u_Material.diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });

    current_used_->SetUniformVec3("u_Material.ambient", glm::vec3{ 0.01f, 0.01f, 0.01f });
    current_used_->SetUniformVec3("u_Material.specular", glm::vec3{ 0.797357, 0.723991, 0.208006 });
    current_used_->SetUniformFloat("u_Material.shininess", 87.2f);

    unshaded_->Use();
    unshaded_->SetUniformVec3("u_Material.diffuse", glm::vec3{ 1, 0, 0 });
    current_used_->Use();

    shader_->SetUniformMat4("u_ProjectionView", glm::identity<glm::mat4>());
    shader_->SetUniformMat4("u_Transform", glm::identity<glm::mat4>());
    glm::vec3 white{ 1.0f, 1.0f, 1.0f };

    shader_->SetUniformVec3("u_Material.Tint", white);

    wireframe_material_ = std::make_shared<Material>(unshaded_);
    material_ = std::make_shared<Material>(shader_);

    material_->SetVector3Property("diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });
    material_->SetVector3Property("ambient", glm::vec3{ 0.01f, 0.01f, 0.01f });
    material_->SetVector3Property("specular", glm::vec3{ 0.797357, 0.723991, 0.208006 });
    material_->SetFloatProperty("shininess", 87.2f);
    wireframe_material_->use_wireframe = true;
    current_material_ = material_;

    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    static_mesh_ = std::make_unique<StaticMesh>("postac.obj", material_);

    camera_rotation_ = glm::quat{ glm::radians(glm::vec3{pitch_, yaw_, 0.0f}) };
    static_mesh_position_ = { 2, 0, -10 };
    std::vector<std::string> animations = std::move(skeletal_mesh_.GetAnimationNames());
    skeletal_mesh_.SetCurrentAnimation(animations[1]);
    RenderCommand::SetClearColor(0.2f, 0.3f, 0.6f);
}

void SandboxGameLayer::OnUpdate(float delta_time) {
    if (IsKeyDown(GLFW_KEY_W)) {
        glm::vec3 world_forward = glm::vec3{ 0, 0, -1 };
        glm::vec3 forward = camera_rotation_ * world_forward * delta_time * move_speed_;
        camera_position_ += forward;
    } else if (IsKeyDown(GLFW_KEY_S)) {
        glm::vec3 world_backward = glm::vec3{ 0, 0, 1 };
        glm::vec3 backward = camera_rotation_ * world_backward * delta_time * move_speed_;
        camera_position_ += backward;
    }

    if (IsKeyDown(GLFW_KEY_E)) {
        yaw_ -= yaw_rotation_rate_ * last_delta_seconds_;
        camera_rotation_ = glm::quat{ glm::radians(glm::vec3{pitch_, yaw_, 0.0f}) };
    } else if (IsKeyDown(GLFW_KEY_Q)) {
        yaw_ += yaw_rotation_rate_ * last_delta_seconds_;
        camera_rotation_ = glm::quat{ glm::radians(glm::vec3{pitch_, yaw_, 0.0f}) };
    }

    if (IsKeyDown(GLFW_KEY_Y)) {
        glm::vec3 world_up = glm::vec3{ 0, 1, 0 };
        camera_position_ += ascend_speed_ * world_up * delta_time;
    } else if (IsKeyDown(GLFW_KEY_H)) {
        glm::vec3 world_down = glm::vec3{ 0, -1, 0 };
        camera_position_ += ascend_speed_ * world_down * delta_time;
    }

    last_delta_seconds_ = delta_time;
}

void SandboxGameLayer::OnRender(float delta_time) {
    Renderer::BeginScene(glm::inverse(glm::translate(glm::identity<glm::mat4>(), camera_position_) * glm::mat4_cast(camera_rotation_)), camera_position_);
    current_used_->SetUniformVec3("u_Material.diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });
    static_mesh_->Render(*current_material_, glm::translate(glm::identity<glm::mat4>(), static_mesh_position_));

    unshaded_->Use();
    unshaded_->SetUniformVec3("u_Material.diffuse", glm::vec3{ 1, 0, 0 });
    Renderer::AddDebugBox(static_mesh_->GetBBoxMin(), static_mesh_->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), static_mesh_position_));
    Renderer::AddDebugBox(static_mesh_->GetBBoxMin(), static_mesh_->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), static_mesh_position_ + glm::vec3{ 10, 0, 0 }));

    Renderer::AddDebugBox(bbox_min_, bbox_max_, glm::translate(glm::identity<glm::mat4>(), glm::vec3{ 10, 2, 10 }));

    skeletal_mesh_.UpdateAnimation(std::chrono::duration_cast<TimeSeconds>(GetNow() - startup_time_).count());
    skeletal_mesh_.Draw(glm::translate(glm::vec3{ 10, 0, 0 }) * glm::scale(glm::vec3{ 0.01f, 0.01f, 0.01f }));
    Renderer::FlushDrawDebug(*unshaded_);

    Renderer::EndScene();
}

bool SandboxGameLayer::OnEvent(const Event& event) {
    if (event.type == EventType::kMouseMoved) {
        glm::vec2 delta = event.mouse_move.mouse_position - event.mouse_move.last_mouse_position;

        yaw_ -= 10 * delta.x * last_delta_seconds_;
        pitch_ -= 10 * delta.y * last_delta_seconds_;

        if (pitch_ < -89) {
            pitch_ = -89;
        }
        if (pitch_ > 89) {
            pitch_ = 89;
        }

        camera_rotation_ = glm::quat{ glm::radians(glm::vec3{pitch_, yaw_, 0.0f}) };
        return true;
    }

    if (event.type == EventType::kMouseButtonPressed) {
        sterring_entity_ = !sterring_entity_;
    }

    if (event.type == EventType::kKeyPressed && event.key.code == GLFW_KEY_P) {
        if (current_material_.get() == wireframe_material_.get()) {
            current_material_ = material_;
        } else {
            current_material_ = wireframe_material_;
        }
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame() {
    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &static_mesh_position_[0], -10, 10);
    ImGui::End();
}
