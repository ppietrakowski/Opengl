#pragma once

#include <engine.h>
#include <glm/gtc/quaternion.hpp>

#include "level.h"

class SandboxGameLayer : public ILayer
{
public:
    SandboxGameLayer();

    // Inherited via IGameLayer
    void Update(Duration delta_time) override;
    void Render(Duration delta_time) override;
    bool OnEvent(const Event& event) override;
    void OnImguiFrame() override;

    virtual std::type_index GetTypeIndex() const override
    {
        return typeid(SandboxGameLayer);
    }


private:
    std::shared_ptr<Shader> default_shader_;
    std::shared_ptr<Shader> debug_shader_;
    std::shared_ptr<Shader> current_used_shader_;

    std::shared_ptr<StaticMesh> static_mesh_;
    glm::vec3 static_mesh_position_;

    glm::vec3 camera_position_;
    glm::quat camera_rotation_;

    std::shared_ptr<Material> default_material_;
    std::shared_ptr<Material> debug_material_;
    std::shared_ptr<Material> current_material_;
    std::shared_ptr<InstancedMesh> instanced_mesh_;

    Actor m_Player;

    float camera_yaw_ = 0.0f;
    float camera_pitch_ = 0.0f;

    Duration last_delta_seconds_;

    float move_speed_ = 20.0f;
    float yaw_rotation_rate_ = 10.0f;
    float pitch_rotation_rate_ = 10.0f;
    float ascend_speed_ = 20.0f;

    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;
    std::shared_ptr<SkeletalMesh> test_skeletal_mesh_;

    Level level_;
    Actor skeletal_mesh_actor_;

private:
    void MoveForward(Actor& player, float axisValue);
    void MoveRight(Actor& player, float axisValue);
    void RotateCamera(Actor& player, glm::vec2 mouseMoveDelta);
};

