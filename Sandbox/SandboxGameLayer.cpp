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
    _cameraRotation{ glm::vec3{0, 0, 0} },
    _cameraPosition{ 0.0f, 0.0f, 0.0f }
{

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

    ELOG_INFO(LOG_GLOBAL, "Loading postac.obj");
    _mesh = std::make_unique<StaticMesh>("postac.obj", _material);

    _cameraRotation = glm::quat{ glm::radians(glm::vec3{_pitch, _yaw, 0.0f}) };
    _position = { 2, 0, -10 };

    RenderCommand::SetClearColor(0.2f, 0.3f, 0.6f);
    boneTransforms.resize(1, glm::identity<glm::mat4>());
    ELOG_INFO(LOG_GLOBAL, "Loading skeleton.vert and skeleton.frag");
    SkeletonShader = Shader::LoadShader("skeleton.vert", "skeleton.frag");
    ELOG_INFO(LOG_GLOBAL, "Loading skeleton.vert and textured.frag");
    _materialTest = std::make_shared<Material>(Shader::LoadShader("skeleton.vert", "textured.frag"));
    ELOG_INFO(LOG_GLOBAL, "Loading skeleton.vert and textured.frag");
    _materialTest->SetTextureProperty("diffuse", std::make_shared<Texture2D>("photomode_30112023_222913.png"));
}

#if 1
std::pair<std::uint32_t, float> getTimeFraction(float dt, std::vector<float>& times)
{
    for (std::uint32_t i = 0; i < times.size() - 1; i++)
    {
        float t = (float)times[i];
        if (dt < t)
        {
            return { i, t };
        }
    }

    return { 0, 0 };
}

#else
std::pair<std::uint32_t, float> getTimeFraction(float dt, std::vector<float>& times)
{
    std::uint32_t segment = 0;
    while (dt > times[segment])
        segment++;
    float start = times[segment - 1];
    float end = times[segment];
    float frac = (dt - start) / (end - start);
    return { segment, frac };
}


#endif

void SandboxGameLayer::getPose(Animation& animation, Joint& skeletion, float dt, std::vector<glm::mat4>& output, glm::mat4& parentTransform, glm::mat4& globalInverseTransform)
{
    BoneAnimationTrack& btt = animation.BoneNamesToTracks[skeletion.Name];
    dt = fmod(dt, animation.Duration);
    std::pair<std::uint32_t, float> fp;
    //calculate interpolated position

    glm::vec3 position{ 0.0f, 0.0f, 0.0f };

    if (!btt.PositionTimestamps.empty())
    {
        fp = getTimeFraction(dt, btt.PositionTimestamps);

        if (btt.PositionTimestamps.size() > 0)
        {
            std::uint32_t index = static_cast<std::uint32_t>(fp.first - 1);
            glm::vec3 position1 = btt.Positions[index];
            glm::vec3 position2 = btt.Positions[fp.first];
            position = glm::mix(position1, position2, fp.second);
        }
        else
        {
            position = btt.Positions[fp.first];
        }
    }

    glm::quat rotation{ glm::vec3{0, 0, 0} };
    if (!btt.RotationsTimestamps.empty())
    {
        //calculate interpolated rotation
        fp = getTimeFraction(dt, btt.RotationsTimestamps);

        if (btt.PositionTimestamps.size() > 0)
        {
            std::uint32_t index = static_cast<std::uint32_t>(fp.first - 1);
            glm::quat rotation1 = btt.Rotations[index];
            glm::quat rotation2 = btt.Rotations[fp.first];
            rotation = glm::slerp(rotation1, rotation2, fp.second);
        }
        else
        {
            rotation = btt.Rotations[0];
        }
    }

    glm::mat4 positionMat = glm::translate(positionMat, position);
    glm::mat4 rotationMat = glm::mat4_cast(rotation);

    glm::mat4 localTransform = positionMat * rotationMat;
    glm::mat4 globalTransform = parentTransform * localTransform;

    output[skeletion.IndexInBoneTransformArray] = globalInverseTransform * globalTransform * skeletion.RelativeTransformMatrix;
    //update values for children bones
    for (Joint& child : skeletion.Children)
    {
        getPose(animation, child, dt, output, globalTransform, globalInverseTransform);
    }

    //std::cout << dt << " => " << position.x << ":" << position.y << ":" << position.z << ":" << std::endl;
}

#if 1
inline glm::mat4 ToGlm(aiMatrix4x4 matrix)
{
    return glm::make_mat4(matrix.Transpose()[0]);
}
#else
inline glm::mat4 ToGlm(aiMatrix4x4 mat)
{
    glm::mat4 m;
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            m[x][y] = mat[y][x];
        }
    }

    return m;
}


#endif

inline glm::vec3 ToGlm(aiVector3D v)
{
    return glm::vec3{ v.x, v.y, v.z };
}

inline glm::vec2 ToGlm(aiVector2D v)
{
    return glm::vec2{ v.x, v.y };
}

inline glm::quat ToGlm(aiQuaternion quat)
{
    glm::quat q;
    q.x = quat.x;
    q.y = quat.y;
    q.z = quat.z;
    q.w = quat.w;

    return q;
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

    if (deltaTime != 0)
    {
        glm::mat4 globalInverseTransform = GlobalInverseTransform;
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



    Renderer::AddDebugBox(BboxMin, BboxMax, glm::translate(glm::identity<glm::mat4>(), glm::vec3{ 10, 2, 10 }));
    Renderer::FlushDrawDebug(*_unshaded);
    mesh.Draw(*_materialTest, deltaTime);

    Renderer::EndScene();
    glLineWidth(1);
}

bool SandboxGameLayer::OnEvent(const Event& event)
{
    if (event.Type == EventType::MouseMoved)
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

void VertexBoneData::AddBoneData(std::uint32_t boneID, float weight)
{
    // find first empty slot
    auto it = std::find(std::begin(weights), std::end(weights), 0.0f);

    if (it != std::end(weights))
    {
        std::ptrdiff_t index = std::distance(std::begin(weights), it);
        bone_ids[index] = boneID;
        weights[index] = weight;
    }
}

struct StbiDeleter
{
    void operator()(std::uint8_t* bytes)
    {
        stbi_image_free(bytes);
    }
};

struct FVertex
{
    glm::vec3 Position{ 0, 0,0 };
    glm::vec3 Normal{ 0, 0, 0 };
    glm::vec2 TextureCoords{ 0, 0 };
    float BoneIDs[kNumBonesPerVertex] = { 0, 0, 0, 0 };
    float BoneWeights[kNumBonesPerVertex] = { 0, 0, 0, 0 };

    static inline constexpr VertexAttribute DataFormat[6] = {
        { 3, PrimitiveVertexType::Float },
        { 3, PrimitiveVertexType::Float },
        { 2, PrimitiveVertexType::Float },
        { kNumBonesPerVertex, PrimitiveVertexType::Float },
        { kNumBonesPerVertex, PrimitiveVertexType::Float }
    };

    FVertex() = default;
    FVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
        Position{ position },
        Normal{ normal },
        TextureCoords{ textureCoords }
    {
    }

    FVertex(const FVertex&) = default;
    FVertex& operator=(const FVertex&) = default;

    bool AddBoneData(std::uint32_t boneID, float weight)
    {
        // find first empty slot
        auto it = std::find(std::begin(BoneWeights), std::end(BoneWeights), 0.0f);

        if (it != std::end(BoneWeights))
        {
            std::ptrdiff_t index = std::distance(std::begin(BoneWeights), it);
            BoneIDs[index] = boneID;
            BoneWeights[index] = weight;

            return true;
        }

        return false;
    }
};

using StbiImageData = std::unique_ptr<std::uint8_t, StbiDeleter>;

struct BoneInfo
{
    std::uint32_t IndexInBoneTransformArray;
    glm::mat4 OffsetMatrix;

    BoneInfo() = default;
    BoneInfo(const BoneInfo&) = default;
    BoneInfo& operator=(const BoneInfo&) = default;
    BoneInfo(std::uint32_t IndexInBoneTransformArray, const glm::mat4& OffsetMatrix)
        : IndexInBoneTransformArray(IndexInBoneTransformArray), OffsetMatrix(OffsetMatrix)
    {
    }
};

bool FindHierarchy(const aiNode* node, Joint& currentJoint, const std::unordered_map<std::string, BoneInfo>& BoneInfos)
{
    auto it = BoneInfos.find(node->mName.C_Str());

    if (it != BoneInfos.end())
    {
        currentJoint.Name = node->mName.C_Str();
        auto& boneInfo = it->second;

        currentJoint.IndexInBoneTransformArray = boneInfo.IndexInBoneTransformArray;
        currentJoint.RelativeTransformMatrix = ToGlm(node->mTransformation);
        currentJoint.BoneOffset = boneInfo.OffsetMatrix;

        for (std::uint32_t i = 0; i < node->mNumChildren; i++)
        {
            Joint child;
            FindHierarchy(node->mChildren[i], child, BoneInfos);
            currentJoint.Children.emplace_back(child);
        }
        return true;
    }
    else
    {
        for (std::uint32_t i = 0; i < node->mNumChildren; i++)
        {
            if (FindHierarchy(node->mChildren[i], currentJoint, BoneInfos))
            {
                return true;
            }
        }
    }

    return false;
}


void getPose(const Animation& animation, const Joint& skeletion, float dt, std::vector<glm::mat4>& output, const glm::mat4& parentTransform, const glm::mat4& globalInverseTransform);

std::uint32_t FindRotation(float AnimationTimeTicks, const std::vector<float>& timestamps)
{
    ASSERT(timestamps.size() > 0);

    for (std::uint32_t i = 0; i < timestamps.size() - 1; i++)
    {
        float t = (float)timestamps[i + 1];
        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}


SkinnedMesh::SkinnedMesh() :
    InitializationTime{ GetNow() }
{
    // maps start index of selected mesh
    std::vector<std::size_t> meshBaseVertexIndices;

    // maps bone name to boneID
    std::unordered_map<std::string, std::uint32_t> boneNameToIndex;

    auto GetBoneID = [&boneNameToIndex](const aiBone* bone)
    {
        std::uint32_t boneID = 0;
        std::string boneName(bone->mName.C_Str());

        auto it = boneNameToIndex.find(boneName);

        if (it != boneNameToIndex.end())
        {
            boneID = it->second;
        }
        else
        {
            boneID = (std::uint32_t)boneNameToIndex.size();
            boneNameToIndex[boneName] = boneID;
        }

        return boneID;
    };

    std::uint32_t totalVertices = 0;
    std::uint32_t totalIndices = 0;
    std::uint32_t totalBones = 0;

    Assimp::Importer importer;

    constexpr std::uint32_t ImportFlags = aiProcess_Triangulate |
        aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs;
    const aiScene* scene = importer.ReadFile("Walking.fbx", ImportFlags);

    CRASH_EXPECTED_NOT_NULL(scene);

    meshBaseVertexIndices.resize(scene->mNumMeshes);

    std::vector<FVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::size_t numIndices = scene->mMeshes[0]->mNumFaces * 3ui64;

    vertices.reserve(scene->mMeshes[0]->mNumVertices);
    indices.reserve(numIndices);

    aiString Path;

    aiTextureMapping mapping;
    ai_real blend;

    for (std::uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        if (scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &Path, &mapping, nullptr, &blend, nullptr, nullptr) == aiReturn_SUCCESS)
        {
            const aiTexture* texture = scene->GetEmbeddedTexture(Path.C_Str());

            if (texture)
            {
                StbiImageData imageData = nullptr;

                std::int32_t width = 0, height = 0, components_per_pixel = 0;
                if (texture->mHeight == 0)
                {
                    imageData.reset(stbi_load_from_memory(reinterpret_cast<std::uint8_t*>(texture->pcData), texture->mWidth, &width, &height, &components_per_pixel, STBI_rgb_alpha));
                }
                else
                {
                    imageData.reset(stbi_load_from_memory(reinterpret_cast<std::uint8_t*>(texture->pcData), texture->mWidth * texture->mHeight, &width, &height, &components_per_pixel, STBI_rgb_alpha));
                }

                textures_.emplace_back(std::make_shared<Texture2D>(imageData.get(), width, height, TextureFormat::Rgba));
            }
        }
    }

    std::unordered_map<std::string, BoneInfo> BoneInfos;

    for (std::uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        meshBaseVertexIndices[i] = totalVertices;

        for (std::uint32_t j = 0; j < mesh->mNumVertices; ++j)
        {
            aiVector3D pos = mesh->mVertices[j];

            aiVector3D normal = mesh->mNormals[j];
            aiVector3D textureCoord = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(textureCoord));
            //vertices.back().TextureID = mesh->mMaterialIndex;
        }

        for (std::uint32_t j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3);

            for (std::uint32_t k = 0; k < 3; ++k)
            {
                indices.emplace_back(face.mIndices[k] + totalIndices);
            }
        }

        for (std::uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            const aiBone* bone = mesh->mBones[boneIndex];
            std::uint32_t boneID = GetBoneID(bone);

            glm::mat4 offsetMatrix = ToGlm(bone->mOffsetMatrix);
            BoneInfos[bone->mName.C_Str()] = BoneInfo{ boneIndex, offsetMatrix };

            for (std::uint32_t j = 0; j < bone->mNumWeights; j++)
            {
                std::uint32_t id = bone->mWeights[j].mVertexId + meshBaseVertexIndices[i];
                float weight = bone->mWeights[j].mWeight;
                if (!vertices[id].AddBoneData(boneID, weight))
                {
                    break;
                }
            }
        }

        totalVertices += mesh->mNumVertices;
        totalIndices += mesh->mNumFaces * 3;
        totalBones += mesh->mNumBones;
    }

    for (std::uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* anim = scene->mAnimations[i];

        Animation animation{};
        if (anim->mTicksPerSecond != 0.0f)
            animation.TicksPerSecond = anim->mTicksPerSecond;
        else
            animation.TicksPerSecond = 1;

        animation.Duration = anim->mDuration;

        for (std::uint32_t i = 0; i < anim->mNumChannels; i++)
        {
            aiNodeAnim* channel = anim->mChannels[i];
            BoneAnimationTrack track;
            for (std::uint32_t j = 0; j < channel->mNumPositionKeys; j++)
            {
                track.AddNewPositionTimestamp(ToGlm(channel->mPositionKeys[j].mValue), channel->mPositionKeys[j].mTime);
            }
            for (std::uint32_t j = 0; j < channel->mNumRotationKeys; j++)
            {
                track.AddNewRotationTimestamp(ToGlm(channel->mRotationKeys[j].mValue), channel->mRotationKeys[j].mTime);
            }

            if (track.Positions.size() > 1 && track.Rotations.size() > 1)
            {
                animation.BoneNamesToTracks[channel->mNodeName.C_Str()] = track;
            }
        }

        _animations[anim->mName.C_Str()] = animation;
    }
    BoneCount = totalBones;

    FindHierarchy(scene->mRootNode, RootJoint, BoneInfos);
    vertexArray_.AddBuffer<FVertex>(vertices, FVertex::DataFormat);
    vertexArray_.SetIndexBuffer(IndexBuffer(indices.data(), static_cast<std::uint32_t>(indices.size())));

    GlobalInversedTransform = glm::inverse(ToGlm(scene->mRootNode->mTransformation));
    BoneTransforms.resize(200, glm::identity<glm::mat4>());
}

void SkinnedMesh::Draw(Shader& shader)
{
    Renderer::Submit(shader, vertexArray_);
}

void SkinnedMesh::Draw(Material& material, float dt)
{
    static float elapsed = 0.0f;
    static bool doneOnce = false;

    elapsed += dt * 40;

    if (!doneOnce)
    {
        material.ShouldCullFaces = false;
        material.UsingTransparency = true;

        doneOnce = true;
    };

    for (std::uint32_t i = 0; i < textures_.size(); ++i)
    {
        std::string name = "diffuse" + std::to_string(i + 1);
        material.SetTextureProperty(name.c_str(), textures_[i]);
    }

    glm::mat4 identity = glm::identity<glm::mat4>();
    CalculateTransform(ticks, RootJoint);
    Renderer::SubmitSkeleton(material, BoneTransforms, BoneCount, vertexArray_, glm::scale(glm::vec3{ 0.1f, 0.1f, 0.1f }));
    ++ticks;
}

void SkinnedMesh::CalculateTransform(float elapsedTime, const Joint& joint, const glm::mat4& parentTransform)
{
    const Animation& animation = _animations.begin()->second;
    glm::mat4 transform = joint.RelativeTransformMatrix;

    float TicksPerSecond = animation.TicksPerSecond != 0 ?
        animation.TicksPerSecond : 25.0f;
    float TimeInTicks = (std::chrono::duration_cast<TimeSeconds>(GetNow()) - this->InitializationTime).count() * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, animation.Duration);

    float dt = AnimationTime;

    auto it = animation.BoneNamesToTracks.find(joint.Name);

    if (it != animation.BoneNamesToTracks.end())
    {
        const BoneAnimationTrack& track = it->second;

        // interpolate
        std::uint32_t positionIndex = FindRotation(dt, track.PositionTimestamps);

        if (track.PositionTimestamps.back() <= dt)
        {
            dt = track.PositionTimestamps[positionIndex];
        }

        std::uint32_t nextPositionIndex = positionIndex + 1;
        float DeltaTime = track.PositionTimestamps[nextPositionIndex] - track.PositionTimestamps[positionIndex];
        float Factor = (dt - track.PositionTimestamps[positionIndex]) / DeltaTime;
        ASSERT(Factor >= 0 && Factor <= 1);

        glm::vec3 position = glm::mix(track.Positions[positionIndex], track.Positions[nextPositionIndex], Factor);
    
        std::uint32_t rotationIndex = FindRotation(dt, track.RotationsTimestamps);
        std::uint32_t nextRotationIndex = rotationIndex + 1;

        if (track.RotationsTimestamps.back() <= dt)
        {
            dt = track.PositionTimestamps[rotationIndex];
        }

        DeltaTime = track.RotationsTimestamps[nextRotationIndex] - track.RotationsTimestamps[rotationIndex];
        Factor = (dt - track.RotationsTimestamps[rotationIndex]) / DeltaTime;
        ASSERT(Factor >= 0 && Factor <= 1);
        glm::quat rotation = glm::mix(track.Rotations[rotationIndex], track.Rotations[nextRotationIndex], Factor);
        transform = glm::translate(position) * glm::mat4_cast(rotation);
    }

    std::uint32_t index = joint.IndexInBoneTransformArray;
    glm::mat4 globalTransform = parentTransform * transform;
    BoneTransforms[index] = GlobalInversedTransform * globalTransform * joint.BoneOffset;

    for (const Joint& child : joint.Children)
    {
        CalculateTransform(elapsedTime, child, globalTransform);
    }
}
