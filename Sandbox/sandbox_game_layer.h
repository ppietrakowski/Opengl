#pragma once

#include <engine.h>
#include <glm/gtc/quaternion.hpp>

#include "level.h"

class SandboxGameLayer : public Layer
{
public:
    SandboxGameLayer(Game* game);

    // Inherited via IGameLayer
    void Update(Duration deltaTime) override;
    void Render() override;
    bool OnEvent(const Event& event) override;
    void OnImguiFrame() override;

    void OnImgizmoFrame() override;

    virtual std::type_index GetTypeIndex() const override
    {
        return typeid(SandboxGameLayer);
    }

private:
    std::shared_ptr<Shader> m_DefaultShader;
    std::shared_ptr<Shader> m_DebugShader;
    std::shared_ptr<Shader> m_CurrentUsedShader;

    std::shared_ptr<Material> m_DebugMaterial;
    std::shared_ptr<InstancedMesh> m_InstancedMesh;
    std::unique_ptr<Skybox> m_Skybox;

    float m_CameraYaw = 0.0f;
    float m_CameraPitch = 0.0f;

    Duration m_LastDeltaSeconds;

    float m_MoveSpeed = 20.0f;
    float m_YawRotationRate = 10.0f;
    float m_PitchRotationRate = 10.0f;
    float m_AscendSpeed = 20.0f;

    std::shared_ptr<SkeletalMesh> m_TestSkeletalMesh;
    Actor m_SelectedActor;

    Level m_Level;
    Game* m_Game{nullptr};

private:
    void MoveForward(Actor& player, float axisValue);
    void MoveRight(Actor& player, float axisValue);
    void RotateCamera(Actor& player, glm::vec2 mouseMoveDelta);

    void InitializeSkeletalMesh();

    void CreateSkeletalActors();

    Actor CreateInstancedMeshActor(const std::string& filePath, const std::shared_ptr<Material>& material);
    void PlaceLightsAndPlayer();
};

