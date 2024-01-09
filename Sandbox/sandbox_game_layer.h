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
    std::shared_ptr<Shader> m_Shader;
    std::shared_ptr<Shader> m_Unshaded;
    std::shared_ptr<Shader> m_CurrentUsed;

    std::shared_ptr<StaticMesh> m_StaticMesh;
    glm::vec3 m_StaticMeshPosition;

    glm::vec3 m_CameraPosition;
    glm::quat m_CameraRotation;

    std::shared_ptr<Material> m_Material;
    std::shared_ptr<Material> m_WireframeMaterial;
    std::shared_ptr<Material> m_CurrentMaterial;
    std::shared_ptr<Material> m_MaterialTest;
    std::shared_ptr<InstancedMesh> m_InstancedMesh;

    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    Duration m_LastDeltaSeconds;
    bool m_bWasPressedLastTime = false;
    bool m_bSterringEntity{false};

    float m_MoveSpeed = 20.0f;
    float m_YawRotationRate = 10.0f;
    float m_PitchRotationRate = 10.0f;
    float m_AscendSpeed = 20.0f;

    glm::vec3 m_BboxMin;
    glm::vec3 m_BboxMax;
    Duration m_StartupTime{GetNow()};
    Duration m_Duration{GetNow()};
    std::shared_ptr<SkeletalMesh> m_SkeletalMesh;

    Level m_Level;
    Actor m_SkeletalMeshActor;
};

