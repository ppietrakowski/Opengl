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

inline constexpr std::int32_t NumBonesPerVertex = 4;
static constexpr const char* DefaultAnimationName = "TPose";

struct SkeletonMeshVertex
{
    glm::vec3 Position{0, 0,0};
    glm::vec3 Normal{0, 0, 0};
    glm::vec2 TextureCoords{0, 0};
    std::int32_t BoneIds[NumBonesPerVertex] = {0, 0, 0, 0};
    float BoneWeights[NumBonesPerVertex] = {0, 0, 0, 0};
    std::uint32_t TextureId{0};

    static inline constexpr VertexAttribute DataFormat[6] = {
        { 3, PrimitiveVertexType::Float },
        { 3, PrimitiveVertexType::Float },
        { 2, PrimitiveVertexType::Float },
        { NumBonesPerVertex, PrimitiveVertexType::Int },
        { NumBonesPerVertex, PrimitiveVertexType::Float },
        { 1, PrimitiveVertexType::UnsignedInt }
    };

    SkeletonMeshVertex() = default;
    SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords);
    SkeletonMeshVertex(const SkeletonMeshVertex&) = default;
    SkeletonMeshVertex& operator=(const SkeletonMeshVertex&) = default;
    bool AddBoneData(std::int32_t boneId, float weight);
};

template <typename T>
struct KeyProperty
{
    T Property;
    float Timestamp;
};

using VectorProperty = KeyProperty<glm::vec3>;
using QuatProperty = KeyProperty<glm::quat>;

class BoneAnimationTrack
{
public:
    void AddNewPositionTimestamp(glm::vec3 position, float Timestamp);
    void AddNewRotationTimestamp(glm::quat rotation, float Timestamp);

    template <typename T>
    T Interpolate(float animationTime) const;

    template<>
    glm::vec3 Interpolate<glm::vec3>(float animationTime) const;

    template<>
    glm::quat Interpolate<glm::quat>(float animationTime) const;

private:
    std::vector<VectorProperty> m_PositionsKeys;
    std::vector<QuatProperty> m_RotationKeys;

    template <typename T>
    std::int32_t GetIndex(float animationTime, const std::vector<KeyProperty<T>>& timestamps) const;
};


struct BoneInfo
{
    std::int32_t BoneTransformIndex;

    /* Matrix that convert vertex to bone space */
    glm::mat4 OffsetMatrix;

    BoneInfo() = default;
    BoneInfo(const BoneInfo&) = default;
    BoneInfo& operator=(const BoneInfo&) = default;
    BoneInfo(std::int32_t boneTransformIndex, const glm::mat4& offsetMatrix) :
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
    std::int32_t BoneTransformIndex{0};

    /* Relative transformation to it's parent */
    glm::mat4 RelativeTransformMatrix{glm::identity<glm::mat4>()};

    /* Matrix that convert vertex to bone space */
    glm::mat4 BoneOffset{glm::identity<glm::mat4>()};
    std::string Name;

    bool AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& bonesInfo);
};

struct SkeletalAnimation
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
    const SkeletalAnimation* TargetAnimation;
    float AnimationTime;
    const Bone* TargetBone;

    const SkeletalAnimation* operator->() const
    {
        ASSERT(TargetAnimation != nullptr);
        return TargetAnimation;
    }
};

class SkeletalMesh
{
    friend struct SkeletalMeshComponent;

public:
    SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material);

    std::vector<std::string> GetAnimationNames() const;

    void GetAnimationFrames(float elapsedTime, const std::string& name, std::vector<glm::mat4>& transforms) const;

    const glm::vec3& GetBboxMin() const
    {
        return m_BboxMin;
    }

    const glm::vec3& GetBboxMax() const
    {
        return m_BboxMax;
    }

    const std::shared_ptr<Material>& GetMaterial() const
    {
        return MainMaterial;
    }

    std::int32_t GetNumBones() const
    {
        return m_NumBones;
    }

    std::shared_ptr<Material> MainMaterial;
    std::vector<std::string> Textures;

private:
    std::shared_ptr<VertexArray> m_VertexArray;
    Bone m_RootBone;
    std::unordered_map<std::string, SkeletalAnimation> m_Animations;
    glm::mat4 m_GlobalInverseTransform;

    std::int32_t m_NumBones;

    glm::vec3 m_BboxMin;
    glm::vec3 m_BboxMax;

private:
    void UpdateAnimation(const std::string& animationName, float elapsedTime, std::vector<glm::mat4>& transforms) const;
    void CalculateTransform(const BoneAnimationUpdateSpecs& updateSpecs, std::vector<glm::mat4>& transforms, const glm::mat4& parentTransform = glm::identity<glm::mat4>()) const;
    std::shared_ptr<Texture2D> LoadTexturesFromMaterial(const aiScene* scene, std::int32_t materialIndex);
    void LoadAnimation(const aiScene* scene, std::int32_t animationIndex);

    void Draw(const std::vector<glm::mat4>& transforms, const glm::mat4& worldTransform);
};

template<>
inline glm::vec3 BoneAnimationTrack::Interpolate(float animationTime) const
{
    if (m_PositionsKeys.size() == 1)
    {
        return m_PositionsKeys[0].Property;
    } else if (!m_PositionsKeys.empty())
    {
        std::int32_t posIndex = GetIndex(animationTime, m_PositionsKeys);
        std::int32_t nextPosIndex = posIndex + 1;

        float deltaTime = m_PositionsKeys[nextPosIndex].Timestamp - m_PositionsKeys[posIndex].Timestamp;
        float factor = (animationTime - m_PositionsKeys[posIndex].Timestamp) / deltaTime;
        ASSERT(factor >= 0 && factor <= 1);

        return glm::mix(m_PositionsKeys[posIndex].Property, m_PositionsKeys[nextPosIndex].Property, factor);
    } else
    {
        return glm::vec3{0, 0, 0};
    }
}

template<>
inline glm::quat BoneAnimationTrack::Interpolate(float animationTime) const
{
    if (m_RotationKeys.size() == 1)
    {
        // not enough keys, use first key as base
        return m_RotationKeys[0].Property;
    } else if (!m_RotationKeys.empty())
    {
        // find time range based on animationTime
        std::int32_t rotIndex = GetIndex(animationTime, m_RotationKeys);
        std::int32_t nextRotIndex = rotIndex + 1;

        float deltaTime = m_RotationKeys[nextRotIndex].Timestamp - m_RotationKeys[rotIndex].Timestamp;
        float factor = (animationTime - m_RotationKeys[rotIndex].Timestamp) / deltaTime;
        ASSERT(factor >= 0 && factor <= 1);
        return glm::mix(m_RotationKeys[rotIndex].Property, m_RotationKeys[nextRotIndex].Property, factor);
    }

    return glm::quat{glm::vec3{0, 0, 0}};
}


template <typename T>
inline std::int32_t BoneAnimationTrack::GetIndex(float animationTime, const std::vector<KeyProperty<T>>& keys) const
{
    ASSERT(keys.size() > 0 && "Called BoneAnimationTrack::GetIndex with track without keys");

    // run binary search to find first Timestamp that is greater than animationTime (max in time range)
    auto it = std::lower_bound(keys.begin(), keys.end(), animationTime, [](const KeyProperty<T>& k, float time) {
        return time > k.Timestamp;
    });

    bool bIsLastAnimTimestamp = it == keys.end();
    if (bIsLastAnimTimestamp)
    {
        return keys.size() > 2 ? static_cast<uint32_t>(keys.size() - 2) : 0;
    }

    std::int32_t leftSideRange = static_cast<std::int32_t>(std::distance(keys.begin(), it));
    ASSERT(leftSideRange >= 0);

    if (leftSideRange != 0)
    {
        // if it's not a first frame of animation, adjust that i now defines left side of range
        // otherwise it's just range {0, 1}
        --leftSideRange;
    }

    return leftSideRange;
}

inline void SkeletalMesh::UpdateAnimation(const std::string& animationName, float elapsedTime, std::vector<glm::mat4>& transforms) const
{
    const SkeletalAnimation& animation = m_Animations.at(animationName);

    // precalculate animation time to
    float timeInTicks = elapsedTime * animation.TicksPerSecond;
    float animationTime = fmod(timeInTicks, animation.Duration);

    // run transform update chain starting from root joint
    CalculateTransform(BoneAnimationUpdateSpecs{&animation, animationTime, &m_RootBone}, transforms);
}


inline void BoneAnimationTrack::AddNewPositionTimestamp(glm::vec3 position, float timestamp)
{
    m_PositionsKeys.emplace_back(VectorProperty{position, timestamp});
}

inline void BoneAnimationTrack::AddNewRotationTimestamp(glm::quat Rotation, float timestamp)
{
    m_RotationKeys.emplace_back(QuatProperty{Rotation, timestamp});
}