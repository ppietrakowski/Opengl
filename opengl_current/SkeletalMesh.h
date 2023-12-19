#pragma once

#include "VertexArray.h"
#include "ErrorMacros.h"
#include "GameLayer.h"
#include "Material.h"

#include <glm/glm.hpp>
#include <span>
#include <string>

#include <memory>
#include <cstdint>
#include <filesystem>

#include <array>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

inline constexpr std::size_t NumBonesPerVertex = 4;

struct SkeletonMeshVertex
{
    glm::vec3 Position{ 0, 0,0 };
    glm::vec3 Normal{ 0, 0, 0 };
    glm::vec2 TextureCoords{ 0, 0 };
    float BoneIDs[NumBonesPerVertex] = { 0, 0, 0, 0 };
    float BoneWeights[NumBonesPerVertex] = { 0, 0, 0, 0 };

    static inline constexpr VertexAttribute DataFormat[6] = {
        { 3, PrimitiveVertexType::Float },
        { 3, PrimitiveVertexType::Float },
        { 2, PrimitiveVertexType::Float },
        { NumBonesPerVertex, PrimitiveVertexType::Float },
        { NumBonesPerVertex, PrimitiveVertexType::Float }
    };

    SkeletonMeshVertex() = default;
    SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords);
    SkeletonMeshVertex(const SkeletonMeshVertex&) = default;
    SkeletonMeshVertex& operator=(const SkeletonMeshVertex&) = default;
    bool AddBoneData(std::uint32_t boneID, float weight);
};

class BoneAnimationTrack
{
public:
    void AddNewPositionTimestamp(glm::vec3 Position, float Timestamp);
    void AddNewRotationTimestamp(glm::quat Rotation, float Timestamp);

    template <typename T>
    T Interpolate(float animationTime) const;

    template<>
    glm::vec3 Interpolate<glm::vec3>(float animationTime) const;

    template<>
    glm::quat Interpolate<glm::quat>(float animationTime) const;

    std::uint32_t GetIndex(float animationTime, const std::vector<float>& timestamps) const;

private:
    std::vector<float> PositionTimestamps;
    std::vector<float> RotationsTimestamps;

    // Positions in specific timestamps (size same as PositionTimestamps)
    std::vector<glm::vec3> Positions;

    // Rotations in specific timestamps (size same as RotationsTimestamps)
    std::vector<glm::quat> Rotations;
};

struct Animation
{
    // Duration in ticks
    float Duration{ 0.0f };
    float TicksPerSecond{ 0.0f };
    std::unordered_map<std::string, BoneAnimationTrack> BoneNamesToTracks;

    glm::mat4 GetBoneTransformOrRelative(const std::string& boneName, float animationTime, glm::mat4 relativeTransform) const;
    glm::mat4 GetBoneTransform(const std::string& boneName, float animationTime) const;
};

struct BoneInfo
{
    std::uint32_t IndexInBoneTransformArray;
    glm::mat4 OffsetMatrix;

    BoneInfo() = default;
    BoneInfo(const BoneInfo&) = default;
    BoneInfo& operator=(const BoneInfo&) = default;
    BoneInfo(std::uint32_t indexInBoneTransformArray, const glm::mat4& offsetMatrix)
        : IndexInBoneTransformArray(indexInBoneTransformArray), OffsetMatrix(offsetMatrix)
    {
    }
};

struct aiNode;

struct Joint
{
    std::vector<Joint> Children;
    std::uint32_t IndexInBoneTransformArray{ 0 };
    glm::mat4 RelativeTransformMatrix{ glm::identity<glm::mat4>() };
    glm::mat4 BoneOffset{ glm::identity<glm::mat4>() };
    std::string Name;

    bool AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& BoneInfos);
};


struct aiScene;

class SkeletalMesh
{
public:
    SkeletalMesh(const std::filesystem::path& path);

    void UpdateAnimation(float elapsedTime) { CalculateTransform(elapsedTime, RootJoint); }
    void Draw(Material& material);

    Joint RootJoint;
    std::unordered_map<std::string, Animation> _animations;
    glm::mat4 GlobalInversedTransform;

    TimeSeconds InitializationTime;
    std::uint32_t BoneCount;

    glm::vec3 bboxMin_;
    glm::vec3 bboxMax_;

private:
    std::vector<std::shared_ptr<Texture2D>> textures_;
    VertexArray vertexArray_;
    std::vector<glm::mat4> boneTransforms_;
    std::vector<SkeletonMeshVertex> vertices_;
    std::vector<std::uint32_t> indices_;

private:
    void CalculateTransform(float elapsedTime, const Joint& joint, const glm::mat4& parentTransform = glm::mat4{ 1.0f });
    void LoadTexturesFromMaterial(const aiScene* scene, uint32_t materialIndex);
    void LoadAnimation(const aiScene* scene, uint32_t animationIndex);

};

void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax);


template<>
inline glm::vec3 BoneAnimationTrack::Interpolate(float animationTime) const
{
    if (Positions.size() == 1)
    {
        return Positions[0];
    }
    else if (!Positions.empty())
    {
        std::uint32_t positionIndex = GetIndex(animationTime, PositionTimestamps);
        std::uint32_t nextPositionIndex = positionIndex + 1;

        float deltaTime = PositionTimestamps[nextPositionIndex] - PositionTimestamps[positionIndex];
        float factor = (animationTime - PositionTimestamps[positionIndex]) / deltaTime;
        ASSERT(factor >= 0 && factor <= 1);

        return glm::mix(Positions[positionIndex], Positions[nextPositionIndex], factor);
    }
    else
    {
        return glm::vec3{ 0, 0, 0 };
    }
}

template<>
inline glm::quat BoneAnimationTrack::Interpolate(float animationTime) const
{
    if (Rotations.size() == 1)
    {
        return Rotations[0];
    }
    else if (!Rotations.empty())
    {
        std::uint32_t rotationIndex = GetIndex(animationTime, RotationsTimestamps);
        std::uint32_t nextRotationIndex = rotationIndex + 1;

        float DeltaTime = RotationsTimestamps[nextRotationIndex] - RotationsTimestamps[rotationIndex];
        float Factor = (animationTime - RotationsTimestamps[rotationIndex]) / DeltaTime;
        ASSERT(Factor >= 0 && Factor <= 1);
        return glm::mix(Rotations[rotationIndex], Rotations[nextRotationIndex], Factor);
    }

    return glm::quat{ glm::vec3{0, 0, 0} };
}