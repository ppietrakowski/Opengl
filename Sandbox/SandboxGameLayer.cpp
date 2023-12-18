#include "SandboxGameLayer.h"

#include "Renderer.h"
#include "Imgui/imgui.h"
#include "ErrorMacros.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>

SandboxGameLayer::SandboxGameLayer() :
    _cameraRotation{ glm::vec3{0, 0, 0} },
    _cameraPosition{ 0.0f, 0.0f, 0.0f }
{

    _shader = Shader::LoadShader("shaders/default.vert", "shaders/default.frag");
    _unshaded = Shader::LoadShader("shaders/default.vert", "shaders/Unshaded.frag");

    _currentUsed = _shader;
    _currentUsed->Use();
    _currentUsed->SetUniformVec3("u_LightColor", glm::vec3{ 1, 1, 1 });
    _currentUsed->SetUniformVec3("u_LightPos", glm::vec3{ -1, 0, -5 });
    _shader->SetUniformVec3("u_Material.diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });

    _currentUsed->SetUniformVec3("u_Material.ambient", glm::vec3{ 0.01f, 0.01f, 0.01f });
    _currentUsed->SetUniformVec3("u_Material.specular", glm::vec3{ 0.797357, 0.723991, 0.208006 });
    _currentUsed->SetUniformFloat("u_Material.shininess", 87.2f);

    _unshaded->Use();
    _unshaded->SetUniformVec3("u_Material.diffuse", glm::vec3{ 1, 0, 0 });
    _currentUsed->Use();

    _shader->SetUniformMat4("u_ProjectionView", glm::identity<glm::mat4>());
    _shader->SetUniformMat4("u_Transform", glm::identity<glm::mat4>());
    glm::vec3 white{ 1.0f, 1.0f, 1.0f };

    _shader->SetUniformVec3("u_Material.Tint", white);

    _wireframeMaterial = std::make_shared<Material>(_unshaded);
    _material = std::make_shared<Material>(_shader);

    _material->SetVector3Property("diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });
    _material->SetVector3Property("ambient", glm::vec3{ 0.01f, 0.01f, 0.01f });
    _material->SetVector3Property("specular", glm::vec3{ 0.797357, 0.723991, 0.208006 });
    _material->SetFloatProperty("shininess", 87.2f);
    _wireframeMaterial->UseWireframe = true;
    _currentMaterial = _material;

    _mesh = std::make_unique<StaticMesh>("postac.obj", _material);
    
    _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    _position = { 2, 0, -10 };

    RenderCommand::SetClearColor(0.2f, 0.3f, 0.6f);

    SkeletonMeshImporter importer{ "Walking.fbx" };

    glBindVertexArray(0);

    VertexBuffer buffer(importer._vertices.data(), importer._vertices.size(), false);
    SkeletonVertexArray.AddBuffer(std::move(buffer), SkeletonMeshVertex::DataFormat);

    IndexBuffer indexBuffer(importer._indices.data(), importer._indices.size());

    SkeletonVertexArray.SetIndexBuffer(std::move(indexBuffer));
    boneTransforms.resize(importer._boneCount, glm::identity<glm::mat4>());
    SkeletonShader = Shader::LoadShader("skeleton.vert", "skeleton.frag");
    animation = importer._animations.begin()->second;

    _root = importer._rootJoint;
    GlobalInverseTransform = importer.GlobalInversedTransform;
}


std::pair<std::uint32_t, float> getTimeFraction(std::vector<float>& times, float& dt)
{
    std::uint32_t segment = 0;

    if (times.size() == 1)
    {
        float start = 0.0f;
        float end = times[0];
        float frac = (dt - start) / (end - start);
        return { 0, frac };
    }

    while (dt > times[segment])
        segment++;
    float start = times[segment - 1];
    float end = times[segment];
    float frac = (dt - start) / (end - start);
    return { segment, frac };
}

void getPose(Animation& animation, Joint& skeletion, float dt, std::vector<glm::mat4>& output, glm::mat4& parentTransform, glm::mat4& globalInverseTransform)
{
    BoneAnimationTrack& btt = animation.BoneNamesToTracks[skeletion.Name];
    dt = fmod(dt, animation.Duration);
    std::pair<std::uint32_t, float> fp;
    //calculate interpolated position

    glm::vec3 position{ 0.0f, 0.0f, 0.0f };

    if (!btt.PositionTimestamps.empty())
    {
        fp = getTimeFraction(btt.PositionTimestamps, dt);

        if (btt.PositionTimestamps.size() > 1)
        {
            glm::vec3 position1 = btt.Positions[fp.first - 1];
            glm::vec3 position2 = btt.Positions[fp.first];
            position = glm::mix(position1, position2, fp.second);
        }
        else
        {
            glm::vec3 position1 = glm::vec3{ 0.0f, 0.0f, 0.0f };
            glm::vec3 position2 = btt.Positions[fp.first];
            position = glm::mix(position1, position2, fp.second);
        }
    }

    glm::quat rotation{ glm::vec3{0, 0, 0} };
    if (!btt.RotationsTimestamps.empty())
    {
        //calculate interpolated rotation
        fp = getTimeFraction(btt.RotationsTimestamps, dt);

        if (btt.PositionTimestamps.size() > 1)
        {
            glm::quat rotation1 = btt.Rotations[fp.first - 1];
            glm::quat rotation2 = btt.Rotations[fp.first];
            rotation = glm::slerp(rotation1, rotation2, fp.second);
        }
        else
        {
            glm::quat rotation1 = glm::quat{ glm::vec3{0.0f} };
            glm::quat rotation2 = btt.Rotations[0];
            rotation = glm::slerp(rotation1, rotation2, fp.second);
        }
        
    }

    glm::mat4 positionMat = glm::mat4(1.0),
        scaleMat = glm::mat4(1.0);


    // calculate localTransform
    positionMat = glm::translate(positionMat, position);
    glm::mat4 rotationMat = glm::mat4_cast(rotation);
    scaleMat = glm::mat4{ 1.0f };

    glm::mat4 localTransform = positionMat * rotationMat * scaleMat;
    glm::mat4 globalTransform = parentTransform * localTransform;

    output[skeletion.IndexInBoneTransformArray] = globalInverseTransform * globalTransform * skeletion.OffsetMatrix;
    //update values for children bones
    for (Joint& child : skeletion.Children)
    {
        getPose(animation, child, dt, output, globalTransform, globalInverseTransform);
    }
    //std::cout << dt << " => " << position.x << ":" << position.y << ":" << position.z << ":" << std::endl;
}

inline glm::mat4 ToGlm(aiMatrix4x4 matrix)
{
    return glm::make_mat4(&matrix.Transpose().a1);
}

inline glm::vec3 ToGlm(aiVector3D v)
{
    return glm::vec3{ v.x, v.y, v.z };
}

inline glm::vec2 ToGlm(aiVector2D v)
{
    return glm::vec2{ v.x, v.y };
}

inline glm::quat ToGlm(aiQuaternion v)
{
    return glm::quat{ v.w, v.x, v.y, v.z };
}

void SandboxGameLayer::OnUpdate(float deltaTime)
{
    static float elapsedTime = 0;

    if (IsKeyDown(GLFW_KEY_W))
    {
        glm::vec3 worldForward = glm::vec3{ 0, 0, -1 };
        glm::vec3 forward = _cameraRotation * worldForward * deltaTime * _moveSpeed;
        _cameraPosition += forward;
    }
    else if (IsKeyDown(GLFW_KEY_S))
    {
        glm::vec3 worldBackward = glm::vec3{ 0, 0, 1 };
        glm::vec3 backward = _cameraRotation * worldBackward * deltaTime * _moveSpeed;
        _cameraPosition += backward;
    }

    if (IsKeyDown(GLFW_KEY_E))
    {
        _yaw -= _yawRotationRate * _lastDeltaSeconds;
        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    }
    else if (IsKeyDown(GLFW_KEY_Q))
    {
        _yaw += _yawRotationRate * _lastDeltaSeconds;
        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    }

    if (IsKeyDown(GLFW_KEY_Y))
    {
        glm::vec3 worldUp = glm::vec3{ 0, 1, 0 };
        _cameraPosition += _ascendSpeed * worldUp * deltaTime;
    }
    else if (IsKeyDown(GLFW_KEY_H))
    {
        glm::vec3 worldDown = glm::vec3{ 0, -1, 0 };
        _cameraPosition += _ascendSpeed * worldDown * deltaTime;
    }

    _lastDeltaSeconds = deltaTime;
    elapsedTime += deltaTime;

    glm::mat4 identity{ 1.0f };

    elapsedTime -= (int)elapsedTime;

    if (deltaTime != 0)
    {
        glm::mat4 globalInverseTransform = GlobalInverseTransform;
        getPose(animation, _root, elapsedTime, boneTransforms, identity, globalInverseTransform);
    }
}

void SandboxGameLayer::OnRender(float deltaTime)
{
    Renderer::BeginScene(glm::inverse(glm::translate(glm::identity<glm::mat4>(), _cameraPosition) * glm::mat4_cast(_cameraRotation)), _cameraPosition);
    _currentUsed->SetUniformVec3("u_Material.diffuse", glm::vec3{ 0.34615f, 0.3143f, 0.0903f });
    _mesh->Render(*_currentMaterial, glm::translate(glm::identity<glm::mat4>(), _position));

    glLineWidth(2);

    _unshaded->Use();
    _unshaded->SetUniformVec3("u_Material.diffuse", glm::vec3{ 1, 0, 0 });
    Renderer::AddDebugBox(_mesh->GetBBoxMin(), _mesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), _position));
    Renderer::AddDebugBox(_mesh->GetBBoxMin(), _mesh->GetBBoxMax(), glm::translate(glm::identity<glm::mat4>(), _position + glm::vec3{ 10, 0, 0 }));

    Renderer::FlushDrawDebug(*_unshaded);

    SkeletonShader->Use();
    SkeletonShader->SetUniformMat4Array("bone_transforms", boneTransforms);
    Renderer::Submit(*SkeletonShader, SkeletonVertexArray, glm::translate(glm::identity<glm::mat4>(), glm::vec3{ 10, 2, 10 }));
    Renderer::EndScene();
    glLineWidth(1);
}

bool SandboxGameLayer::OnEvent(const Event& event)
{
    if (0 && event.Type == EventType::MouseMoved)
    {
        glm::vec2 delta = event.MouseMove.MousePosition - event.MouseMove.LastMousePosition;

        _yaw -= 10 * delta.x * _lastDeltaSeconds;
        _pitch -= 10 * delta.y * _lastDeltaSeconds;

        if (_pitch < -89)
        {
            _pitch = -89;
        }
        if (_pitch > 89)
        {
            _pitch = 89;
        }

        _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
        return true;
    }

    if (event.Type == EventType::MouseButtonPressed)
    {
        _sterringEntity = !_sterringEntity;
    }

    if (event.Type == EventType::KeyPressed && event.Key.Code == GLFW_KEY_P) 
    {
        if (_currentMaterial.get() == _wireframeMaterial.get())
        {
            _currentMaterial = _material;
        }
        else
        {
            _currentMaterial = _wireframeMaterial;
        }
    }

    return false;
}

void SandboxGameLayer::OnImguiFrame()
{
    ImGui::Begin("Transform");
    ImGui::SliderFloat3("Position", &_position[0], -10, 10);
    ImGui::End();
}
