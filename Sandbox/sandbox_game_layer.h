#pragma once

#include <engine.h>
#include <glm/gtc/quaternion.hpp>

#include "level.h"

class SandboxGameLayer : public Layer
{
public:
    SandboxGameLayer(Game *game);

    // Inherited via IGameLayer
    void Update(Duration delta_time) override;
    void Render(Duration delta_time) override;
    bool OnEvent(const Event& event) override;
    void OnImguiFrame() override;

    void OnImgizmoFrame() override;

    virtual std::type_index GetTypeIndex() const override
    {
        return typeid(SandboxGameLayer);
    }

private:
    std::shared_ptr<Shader> default_shader_;
    std::shared_ptr<Shader> debug_shader_;
    std::shared_ptr<Shader> current_used_shader_;

    glm::vec3 camera_position_;
    glm::quat camera_rotation_;

    std::shared_ptr<Material> debug_material_;
    std::shared_ptr<InstancedMesh> instanced_mesh_;
    std::unique_ptr<Skybox> skybox_;

    float camera_yaw_ = 0.0f;
    float camera_pitch_ = 0.0f;

    Duration last_delta_seconds_;

    float move_speed_ = 20.0f;
    float yaw_rotation_rate_ = 10.0f;
    float pitch_rotation_rate_ = 10.0f;
    float ascend_speed_ = 20.0f;

    std::shared_ptr<SkeletalMesh> test_skeletal_mesh_;
    Actor selected_actor;

    Level level_;
    Game* game_{nullptr};

private:
    void MoveForward(Actor& player, float axis_value);
    void MoveRight(Actor& player, float axis_value);
    void RotateCamera(Actor& player, glm::vec2 mouse_move_delta);

    void InitializeSkeletalMesh();

    void CreateSkeletalActors();

    Actor CreateInstancedMeshActor(const std::string& file_path, const std::shared_ptr<Material>& material);
    void PlaceLightsAndPlayer();
};

