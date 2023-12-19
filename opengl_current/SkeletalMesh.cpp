#include "SkeletalMesh.h"
#include "Renderer.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

inline glm::mat4 ToGlm(aiMatrix4x4 matrix)
{
    return glm::make_mat4(matrix.Transpose()[0]);
}

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
    return glm::quat{ quat.w, quat.x, quat.y, quat.z };
}


struct StbiDeleter
{
    void operator()(std::uint8_t* bytes)
    {
        stbi_image_free(bytes);
    }
};

using StbiImageData = std::unique_ptr<std::uint8_t, StbiDeleter>;


void BoneAnimationTrack::AddNewPositionTimestamp(glm::vec3 Position, float Timestamp)
{
    Positions.emplace_back(Position);
    PositionTimestamps.emplace_back(Timestamp);
}

void BoneAnimationTrack::AddNewRotationTimestamp(glm::quat Rotation, float Timestamp)
{
    Rotations.emplace_back(Rotation);
    RotationsTimestamps.push_back(Timestamp);
}


std::uint32_t BoneAnimationTrack::GetIndex(float animationTime, const std::vector<float>& timestamps) const
{
    ASSERT(timestamps.size() > 0);

    for (std::uint32_t i = 0; i < timestamps.size() - 1; i++)
    {
        float t = (float)timestamps[i + 1];
        if (animationTime < t)
        {
            return i;
        }
    }

    return 0;
}

glm::mat4 Animation::GetBoneTransformOrRelative(const std::string& boneName, float animationTime, glm::mat4 relativeTransform) const
{
    glm::mat4 transform = GetBoneTransform(boneName, animationTime);

    if (transform == glm::mat4{ 1.0f })
    {
        return relativeTransform;
    }

    return transform;
}

glm::mat4 Animation::GetBoneTransform(const std::string& boneName, float animationTime) const
{
    auto it = BoneNamesToTracks.find(boneName);

    if (it != BoneNamesToTracks.end())
    {
        const BoneAnimationTrack& track = it->second;

        // interpolate
        glm::vec3 position = track.Interpolate<glm::vec3>(animationTime);
        glm::quat rotation = track.Interpolate<glm::quat>(animationTime);
        return glm::translate(position) * glm::mat4_cast(rotation);
    }

    return glm::mat4{ 1.0f };
}

bool Joint::AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& BoneInfos)
{
    auto it = BoneInfos.find(node->mName.C_Str());

    if (it != BoneInfos.end())
    {
        Name = node->mName.C_Str();
        const BoneInfo& boneInfo = it->second;

        IndexInBoneTransformArray = boneInfo.IndexInBoneTransformArray;
        RelativeTransformMatrix = ToGlm(node->mTransformation);
        BoneOffset = boneInfo.OffsetMatrix;

        for (std::uint32_t i = 0; i < node->mNumChildren; i++)
        {
            Joint child;
            child.AssignHierarchy(node->mChildren[i], BoneInfos);
            Children.emplace_back(child);
        }
        return true;
    }
    else
    {
        for (std::uint32_t i = 0; i < node->mNumChildren; i++)
        {
            if (AssignHierarchy(node->mChildren[i], BoneInfos))
            {
                return true;
            }
        }
    }

    return false;
}

SkeletonMeshVertex::SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
    Position{ position },
    Normal{ normal },
    TextureCoords{ textureCoords }
{
}

bool SkeletonMeshVertex::AddBoneData(std::uint32_t boneID, float weight)
{
    // find first empty slot
    auto it = std::find(std::begin(BoneWeights), std::end(BoneWeights), 0.0f);

    if (it != std::end(BoneWeights))
    {
        std::ptrdiff_t index = std::distance(std::begin(BoneWeights), it);
        BoneIDs[index] = (float)boneID;
        BoneWeights[index] = weight;

        return true;
    }

    return false;
}

SkeletalMesh::SkeletalMesh(const std::filesystem::path& path) :
    InitializationTime{ GetNow() }
{
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
        aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs
        ;

    const aiScene* scene = importer.ReadFile(path.string(), ImportFlags);
    CRASH_EXPECTED_NOT_NULL(scene);

    std::uint32_t numIndices = scene->mMeshes[0]->mNumFaces * 3u;

    vertices_.reserve(scene->mMeshes[0]->mNumVertices);
    indices_.reserve(numIndices);

    for (std::uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        LoadTexturesFromMaterial(scene, i);
    }

    std::unordered_map<std::string, BoneInfo> BoneInfos;

    for (std::uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];

        for (std::uint32_t j = 0; j < mesh->mNumVertices; ++j)
        {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D textureCoord = mesh->mTextureCoords[0][j];

            vertices_.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(textureCoord));
        }

        for (std::uint32_t j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3);

            for (std::uint32_t k = 0; k < 3; ++k)
            {
                indices_.emplace_back(face.mIndices[k] + totalIndices);
            }
        }

        for (std::uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            const aiBone* bone = mesh->mBones[boneIndex];
            std::uint32_t boneID = GetBoneID(bone);

            glm::mat4 offsetMatrix = ToGlm(bone->mOffsetMatrix);
            BoneInfos[bone->mName.C_Str()] = BoneInfo{ boneIndex, offsetMatrix };
            std::string s{ bone->mName.C_Str() };

            for (std::uint32_t j = 0; j < bone->mNumWeights; j++)
            {
                std::uint32_t id = bone->mWeights[j].mVertexId + totalVertices;
                float weight = bone->mWeights[j].mWeight;

                if (!vertices_[id].AddBoneData(boneID, weight))
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
        LoadAnimation(scene, i);
    }

    BoneCount = totalBones;

    RootJoint.AssignHierarchy(scene->mRootNode, BoneInfos);
    vertexArray_.AddBuffer<SkeletonMeshVertex>(vertices_, SkeletonMeshVertex::DataFormat);
    vertexArray_.SetIndexBuffer(IndexBuffer(indices_.data(), static_cast<std::uint32_t>(indices_.size())));

    GlobalInversedTransform = glm::inverse(ToGlm(scene->mRootNode->mTransformation));
    boneTransforms_.resize(totalBones, glm::identity<glm::mat4>());

    FindAabCollision(vertices_, bboxMin_, bboxMax_);
    bboxMin_.x *= 0.2f;
    bboxMax_.x *= 0.2f;
}

void SkeletalMesh::LoadAnimation(const aiScene* scene, uint32_t animationIndex)
{
    const aiAnimation* anim = scene->mAnimations[animationIndex];

    Animation animation{};

    if (anim->mTicksPerSecond != 0.0f)
    {
        animation.TicksPerSecond = static_cast<float>(anim->mTicksPerSecond);
    }
    else
    {
        animation.TicksPerSecond = 1.0f;
    }

    animation.Duration = static_cast<float>(anim->mDuration);

    for (std::uint32_t i = 0; i < anim->mNumChannels; i++)
    {
        const aiNodeAnim* channel = anim->mChannels[i];
        BoneAnimationTrack track;

        for (std::uint32_t j = 0; j < channel->mNumPositionKeys; j++)
        {
            track.AddNewPositionTimestamp(ToGlm(channel->mPositionKeys[j].mValue), static_cast<float>(channel->mPositionKeys[j].mTime));
        }
        for (std::uint32_t j = 0; j < channel->mNumRotationKeys; j++)
        {
            track.AddNewRotationTimestamp(ToGlm(channel->mRotationKeys[j].mValue), static_cast<float>(channel->mRotationKeys[j].mTime));
        }

        animation.BoneNamesToTracks[channel->mNodeName.C_Str()] = track;
    }

    _animations[anim->mName.C_Str()] = animation;
}

void SkeletalMesh::Draw(Material& material)
{
    static bool doneOnce = false;

    if (!doneOnce)
    {
        material.ShouldCullFaces = false;
        material.UsingTransparency = true;


        for (std::uint32_t i = 0; i < textures_.size(); ++i)
        {
            std::string name = "diffuse" + std::to_string(i + 1);
            material.SetTextureProperty(name.c_str(), textures_[i]);
        }

        doneOnce = true;
    }

    UpdateAnimation((std::chrono::duration_cast<TimeSeconds>(GetNow()) - this->InitializationTime).count());
    Renderer::AddDebugBox(bboxMin_, bboxMax_, glm::scale(glm::vec3{ 0.01f, 0.01f, 0.01f }));
    Renderer::SubmitSkeleton(material, boneTransforms_, BoneCount, vertexArray_, glm::scale(glm::vec3{ 0.01f, 0.01f, 0.01f }));
}

void SkeletalMesh::CalculateTransform(float elapsedTime, const Joint& joint, const glm::mat4& parentTransform)
{
    const Animation& animation = _animations.begin()->second;

    float ticksPerSecond = animation.TicksPerSecond;
    float timeInTicks = elapsedTime * ticksPerSecond;
    float animationTime = fmod(timeInTicks, animation.Duration);

    glm::mat4 transform = animation.GetBoneTransformOrRelative(joint.Name, animationTime, joint.RelativeTransformMatrix);

    std::uint32_t index = joint.IndexInBoneTransformArray;
    glm::mat4 globalTransform = parentTransform * transform;
    boneTransforms_[index] = GlobalInversedTransform * globalTransform * joint.BoneOffset;

    for (const Joint& child : joint.Children)
    {
        CalculateTransform(elapsedTime, child, globalTransform);
    }
}

void SkeletalMesh::LoadTexturesFromMaterial(const aiScene* scene, uint32_t materialIndex)
{
    aiString path;

    if (scene->mMaterials[materialIndex]->GetTexture(aiTextureType_DIFFUSE, 0,
        &path, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS)
    {
        const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());

        if (texture != nullptr)
        {
            StbiImageData imageData;

            std::int32_t width = 0, height = 0, numComponents = 0;

            bool isCompressed = texture->mHeight == 0;

            if (isCompressed)
            {
                imageData.reset(stbi_load_from_memory(reinterpret_cast<std::uint8_t*>(texture->pcData),
                    texture->mWidth, &width, &height,
                    &numComponents, STBI_rgb_alpha));
            }
            else
            {
                imageData.reset(stbi_load_from_memory(reinterpret_cast<std::uint8_t*>(texture->pcData),
                    texture->mWidth * texture->mHeight,
                    &width, &height, &numComponents, STBI_rgb_alpha));
            }

            textures_.emplace_back(std::make_shared<Texture2D>(imageData.get(), width, height, TextureFormat::Rgba));
        }
    }
}

void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax)
{
    // assume mesh has infinite bounds
    outBoxMin = glm::vec3{ std::numeric_limits<float>::max() };
    outBoxMax = glm::vec3{ std::numeric_limits<float>::min() };

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const glm::vec3* vertex = &vertices[i].Position;

        if (vertex->x < outBoxMin.x)
        {
            outBoxMin.x = vertex->x;
        }
        if (vertex->y < outBoxMin.y)
        {
            outBoxMin.y = vertex->y;
        }
        if (vertex->z < outBoxMin.z)
        {
            outBoxMin.z = vertex->z;
        }

        if (vertex->x > outBoxMax.x)
        {
            outBoxMax.x = vertex->x;
        }
        if (vertex->y > outBoxMax.y)
        {
            outBoxMax.y = vertex->y;
        }
        if (vertex->z > outBoxMax.z)
        {
            outBoxMax.z = vertex->z;
        }
    }
}
