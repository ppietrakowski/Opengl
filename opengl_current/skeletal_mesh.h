#pragma once

#include "vertex_array.h"
#include "error_macros.h"
#include "game_layer.h"
#include "material.h"

#include <glm/glm.hpp>
#include <span>
#include <string>

#include <memory>
#include <cstdint>
#include <filesystem>

#include <array>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

#include <chrono>

inline constexpr std::size_t kNumBonesPerVertex = 4;

struct SkeletonMeshVertex
{
    glm::vec3 Position{0, 0,0};
    glm::vec3 Normal{0, 0, 0};
    glm::vec2 TextureCoords{0, 0};
    int32_t BoneIds[kNumBonesPerVertex] = {0, 0, 0, 0};
    float BoneWeights[kNumBonesPerVertex] = {0, 0, 0, 0};
    uint32_t TextureId{0};

    static inline constexpr VertexAttribute kDataFormat[6] = {
        { 3, PrimitiveVertexType::kFloat },
        { 3, PrimitiveVertexType::kFloat },
        { 2, PrimitiveVertexType::kFloat },
        { kNumBonesPerVertex, PrimitiveVertexType::kInt },
        { kNumBonesPerVertex, PrimitiveVertexType::kFloat },
        { 1, PrimitiveVertexType::kUnsignedInt }
    };

    SkeletonMeshVertex() = default;
    SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords);
    SkeletonMeshVertex(const SkeletonMeshVertex&) = default;
    SkeletonMeshVertex& operator=(const SkeletonMeshVertex&) = default;
    bool AddBoneData(int32_t boneId, float weight);
};

template <typename T>
struct KeyProperty
{
    T Property;
    float Timestamp;
};

class BoneAnimationTrack
{
public:
    void AddNewPositionTimestamp(glm::vec3 position, float timestamp);
    void AddNewRotationTimestamp(glm::quat rotation, float timestamp);

    template <typename T>
    T Interpolate(float animationTime) const;

    template<>
    glm::vec3 Interpolate<glm::vec3>(float animationTime) const;

    template<>
    glm::quat Interpolate<glm::quat>(float animationTime) const;

private:
    std::vector<KeyProperty<glm::vec3>> m_Positions;
    std::vector<KeyProperty<glm::quat>> m_Rotations;

    template <typename T>
    int32_t GetIndex(float animationTime, const std::vector<KeyProperty<T>>& timestamps) const;
};


struct BoneInfo
{
    int32_t BoneTransformIndex;

    /* Matrix that convert vertex to bone space */
    glm::mat4 OffsetMatrix;

    BoneInfo() = default;
    BoneInfo(const BoneInfo&) = default;
    BoneInfo& operator=(const BoneInfo&) = default;
    BoneInfo(int32_t boneTransformIndex, const glm::mat4& offsetMatrix) :
        BoneTransformIndex{boneTransformIndex},
        OffsetMatrix{offsetMatrix}
    {
    }
};

struct aiNode;

struct Bone
{
    std::vector<Bone> Children;

    /* Index in bone_transform_ array */
    int32_t BoneTransformIndex{0};

    /* Relative transformation to it's parent */
    glm::mat4 RelativeTransformMatrix{glm::identity<glm::mat4>()};

    /* Matrix that convert vertex to bone space */
    glm::mat4 BoneOffset{glm::identity<glm::mat4>()};
    std::string Name;

    bool AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& bonesInfo);
};

struct Animation
{
    // Duration in ticks
    float Duration{0.0f};
    float TicksPerSecond{0.0f};

    // bone name mapped to animation track
    std::unordered_map<std::string, BoneAnimationTrack> BoneNameToTracks;

    glm::mat4 GetBoneTransformOrRelative(const Bone& bone, float animationTime) const;
};

struct aiScene;

struct BoneAnimationUpdateSpecs
{
    const Animation* TargetAnimation;
    float AnimationTime;
    const Bone* Joint;

    const Animation* operator->() const
    {
        ASSERT(TargetAnimation != nullptr);
        return TargetAnimation;
    }
};

class SkeletalMesh
{
public:
    SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material);

    void UpdateAnimation(float elapsedTime);

    void Draw(const glm::mat4& transform);

    void SetCurrentAnimation(const std::string& animationName);
    std::vector<std::string> GetAnimationNames() const;

    bool bShouldDrawDebugBounds{false};

private:
    std::shared_ptr<IVertexArray> m_VertexArray;
    std::vector<glm::mat4> m_BoneTransforms;
    Bone m_RootBone;
    std::unordered_map<std::string, Animation> m_Animations;
    glm::mat4 m_GlobalInverseTransform;

    int32_t m_NumBones;

    glm::vec3 m_BboxMin;
    glm::vec3 m_BboxMax;
    std::shared_ptr<Material> m_Material;

    std::string m_CurrentAnimationName;

private:
    void CalculateTransform(const BoneAnimationUpdateSpecs& updateSpecs, const glm::mat4& parentTransform = glm::identity<glm::mat4>());
    std::shared_ptr<ITexture2D> LoadTexturesFromMaterial(const aiScene* scene, int32_t materialIndex);
    void LoadAnimation(const aiScene* scene, int32_t animationIndex);
};

template<>
inline glm::vec3 BoneAnimationTrack::Interpolate(float animationTime) const
{
    if (m_Positions.size() == 1)
    {
        return m_Positions[0].Property;
    }
    else if (!m_Positions.empty())
    {
        int32_t positionIndex = GetIndex(animationTime, m_Positions);
        int32_t nextPositionIndex = positionIndex + 1;

        float deltaTime = m_Positions[nextPositionIndex].Timestamp - m_Positions[positionIndex].Timestamp;
        float factor = (animationTime - m_Positions[positionIndex].Timestamp) / deltaTime;
        ASSERT(factor >= 0 && factor <= 1);

        return glm::mix(m_Positions[positionIndex].Property, m_Positions[nextPositionIndex].Property, factor);
    }
    else
    {
        return glm::vec3{0, 0, 0};
    }
}

template<>
inline glm::quat BoneAnimationTrack::Interpolate(float animationTime) const
{
    if (m_Rotations.size() == 1)
    {
        // not enough keys, use first key as base
        return m_Rotations[0].Property;
    }
    else if (!m_Rotations.empty())
    {
        // find time range based on animationTime
        int32_t rotationIndex = GetIndex(animationTime, m_Rotations);
        int32_t nextRotationIndex = rotationIndex + 1;

        float deltaTime = m_Rotations[nextRotationIndex].Timestamp - m_Rotations[rotationIndex].Timestamp;
        float factor = (animationTime - m_Rotations[rotationIndex].Timestamp) / deltaTime;
        ASSERT(factor >= 0 && factor <= 1);
        return glm::mix(m_Rotations[rotationIndex].Property, m_Rotations[nextRotationIndex].Property, factor);
    }

    return glm::quat{glm::vec3{0, 0, 0}};
}


template <typename T>
inline int32_t BoneAnimationTrack::GetIndex(float animationTime, const std::vector<KeyProperty<T>>& keys) const
{
    ASSERT(keys.size() > 0 && "Called BoneAnimationTrack::GetIndex with track without keys");

    // run binary search to find first timestamp that is greater than animationTime (max in time range)
    auto it = std::lower_bound(keys.begin(), keys.end(), animationTime, [](const KeyProperty<T>& k, float time) {
        return time > k.Timestamp;
    });

    bool bIsLastAnimationTimestamp = it == keys.end();
    if (bIsLastAnimationTimestamp)
    {
        return keys.size() > 2 ? static_cast<uint32_t>(keys.size() - 2) : 0;
    }

    int32_t leftSideRange = static_cast<int32_t>(std::distance(keys.begin(), it));
    ASSERT(leftSideRange >= 0);

    if (leftSideRange != 0)
    {
        // if it's not a first frame of animation, adjust that i now defines left side of range
        // otherwise it's just range {0, 1}
        --leftSideRange;
    }

    return leftSideRange;
}

inline void SkeletalMesh::UpdateAnimation(float elapsedTime)
{
    const Animation& animation = m_Animations.at(m_CurrentAnimationName);

    // precalculate animation time to
    float ticksPerSecond = animation.TicksPerSecond;
    float timeInTicks = elapsedTime * ticksPerSecond;
    float animationTime = fmod(timeInTicks, animation.Duration);

    // run transform update chain starting from root joint
    CalculateTransform(BoneAnimationUpdateSpecs{&animation, animationTime, &m_RootBone});
}


inline void BoneAnimationTrack::AddNewPositionTimestamp(glm::vec3 position, float timestamp)
{
    m_Positions.emplace_back(KeyProperty<glm::vec3>{position, timestamp});
}

inline void BoneAnimationTrack::AddNewRotationTimestamp(glm::quat Rotation, float timestamp)
{
    m_Rotations.emplace_back(KeyProperty<glm::quat>{Rotation, timestamp});
}