#pragma once
#include "GameLayer.h"

#include "Shader.h"
#include "Mesh.h"
#include "SkeletalMesh.h"

#include <glm/gtc/quaternion.hpp>

class SandboxGameLayer : public Layer {
public:
    SandboxGameLayer();

    // Inherited via IGameLayer
    void OnUpdate(float delta_time) override;
    void OnRender(float delta_time) override;
    bool OnEvent(const Event& event) override;
    void OnImguiFrame() override;

    virtual std::type_index GetTypeIndex() const override {
        return typeid(SandboxGameLayer);
    }

private:
    std::shared_ptr<Shader> shader_;
    std::shared_ptr<Shader> unshaded_;
    std::shared_ptr<Shader> current_used_;

    std::unique_ptr<StaticMesh> static_mesh_;
    glm::vec3 static_mesh_position_;

    glm::vec3 camera_position_;
    glm::quat camera_rotation_;

    std::shared_ptr<Material> material_;
    std::shared_ptr<Material> wireframe_material_;
    std::shared_ptr<Material> current_material_;
    std::shared_ptr<Material> material_test_;

    float yaw_ = -90.0f;
    float pitch_ = 0.0f;

    float last_delta_seconds_{ 0.0f };
    bool was_pressed_last_time_ = false;
    bool sterring_entity_{ false };

    float move_speed_ = 20.0f;
    float yaw_rotation_rate_ = 80.0f;
    float ascend_speed_ = 20.0f;

    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;
    std::chrono::milliseconds startup_time_{ GetNow() };
    SkeletalMesh skeletal_mesh_;
};

