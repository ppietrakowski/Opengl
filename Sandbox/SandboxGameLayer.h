#pragma once
#include "GameLayer.h"

#include "Shader.h"
#include "Mesh.h"

#include <glm/gtc/quaternion.hpp>

constexpr inline std::size_t kNumBonesPerVertex = 4;

class SkinnedMesh
{
public:
    SkinnedMesh();

    void UpdateAnimation(float elapsedTime) { CalculateTransform(elapsedTime, RootJoint); }

    void Draw(Material& material, float dt);

    Joint RootJoint;
    std::unordered_map<std::string, Animation> _animations;
    glm::mat4 GlobalInversedTransform;

    TimeSeconds InitializationTime;
    std::uint32_t BoneCount;

private:
    std::vector<std::shared_ptr<Texture2D>> textures_;
    VertexArray vertexArray_;
    std::vector<glm::mat4> BoneTransforms;

private:
    void CalculateTransform(float elapsedTime, const Joint& joint, const glm::mat4& parentTransform = glm::mat4{ 1.0f });
};

class SandboxGameLayer : public Layer
{
public:
    SandboxGameLayer();

    // Inherited via IGameLayer
    void OnUpdate(float deltaTime) override;
    void OnRender(float deltaTime) override;
    bool OnEvent(const Event& event) override;
    void OnImguiFrame() override;

    virtual std::type_index GetTypeIndex() const override
    {
        return typeid(SandboxGameLayer);
    }

private:
    std::shared_ptr<Shader> _shader;
    std::shared_ptr<Shader> _unshaded;
    std::shared_ptr<Shader> _currentUsed;

    std::unique_ptr<StaticMesh> _mesh;
    glm::vec3 _position;

    glm::vec3 _cameraPosition;
    glm::quat _cameraRotation;

    std::shared_ptr<Material> _material;
    std::shared_ptr<Material> _wireframeMaterial;
    std::shared_ptr<Material> _currentMaterial;
    std::shared_ptr<Material> _materialTest;

    float _yaw = -90.0f;
    float _pitch = 0.0f;

    float _lastDeltaSeconds{ 0.0f };
    bool _wasPressedLastTime = false;
    bool _sterringEntity{ false };
    
    float _moveSpeed = 20.0f;
    float _yawRotationRate = 80.0f;
    float _ascendSpeed = 20.0f;

    glm::vec3 BboxMin;
    glm::vec3 BboxMax;

    SkinnedMesh mesh;
};

