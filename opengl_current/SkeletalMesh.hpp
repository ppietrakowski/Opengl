#pragma once

#include "VertexArray.hpp"
#include "ErrorMacros.hpp"
#include "GameLayer.hpp"
#include "Material.hpp"

#include "Box.hpp"

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

inline constexpr int NumBonesPerVertex = 4;
inline constexpr const char* DefaultAnimationName = "TPose";

struct SkeletonMeshVertex
{
    glm::vec3 Position{0, 0,0};
    glm::vec3 Normal{0, 0, 0};
    glm::vec2 TextureCoords{0, 0};
    int32_t BoneIds[NumBonesPerVertex] = {0, 0, 0, 0};
    float BoneWeights[NumBonesPerVertex] = {0, 0, 0, 0};
    uint32_t TextureId{0};

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

    bool AddBoneData(int boneId, float weight);
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
    void AddNewPositionTimestamp(glm::vec3 position, float timestamp);
    void AddNewRotationTimestamp(glm::quat rotation, float timestamp);

    template <typename T>
    T Interpolate(float animationTime) const;

    template<>
    glm::vec3 Interpolate<glm::vec3>(float animationTime) const;

    template<>
    glm::quat Interpolate<glm::quat>(float animationTime) const;

private:
    std::vector<VectorProperty> m_PositionKeys;
    std::vector<QuatProperty> m_RotationKeys;

    template <typename T>
    size_t GetIndex(float animationTime, const std::vector<KeyProperty<T>>& timestamps) const;
};


struct BoneInfo
{
    int BoneTransformIndex;

    /* Matrix that convert vertex to bone space */
    glm::mat4 OffsetMatrix;

    BoneInfo() = default;
    BoneInfo(const BoneInfo&) = default;
    BoneInfo& operator=(const BoneInfo&) = default;
    BoneInfo(int boneTransformIndex, const glm::mat4& offsetMatrix);
};

struct aiNode;

struct Bone
{
    std::vector<Bone> Children;

    /* Index in bone_transform_ array */
    int BoneTransformIndex{0};

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

struct BoneAnimationUpdateArgs
{
    const SkeletalAnimation* UsedAnimation;
    float AnimationTime;
    const Bone* TargetBone;
    std::span<glm::mat4> BoneTransforms;
    glm::mat4 ParentTransform{1.0f};

    const SkeletalAnimation* operator->() const;
    void UpdateTransformAt(int index, const glm::mat4& transform) const;
};

struct AnimationUpdateArgs
{
    float ElapsedTime;
    std::string AnimationName;
    std::span<glm::mat4> Transforms;
};

class SkeletalMesh
{
    friend struct SkeletalMeshComponent;

public:
    SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material);

    std::vector<std::string> GetAnimationNames() const;

    void GetAnimationFrames(const AnimationUpdateArgs& updateArgs) const;

    const glm::vec3& GetBboxMin() const;
    const glm::vec3& GetBboxMax() const;

    const std::shared_ptr<Material>& GetMaterial() const;

    uint32_t GetNumBones() const;

    std::shared_ptr<Material> MainMaterial;

    Box GetBoundingBox() const
    {
        return m_BoundingBox;
    }

    std::vector<std::string> TextureNames;
    const std::string& GetPath() const
    {
        return m_Path;
    }

public:
    const VertexArray& GetVertexArray() const
    {
        return m_VertexArray;
    }

private:
    VertexArray m_VertexArray;
    Bone m_RootBone;
    std::unordered_map<std::string, SkeletalAnimation> m_Animations;
    glm::mat4 m_GlobalInverseTransform;
    uint32_t m_NumBones;
    std::string m_Path;
    Box m_BoundingBox;

private:
    void UpdateAnimation(const AnimationUpdateArgs& updateArgs) const;
    void CalculateTransform(const BoneAnimationUpdateArgs& updateArgs) const;
    std::shared_ptr<Texture2D> LoadTexturesFromMaterial(const aiScene* scene, int materialIndex);
    void LoadAnimation(const aiScene* scene, int animationIndex);
};

template<>
inline glm::vec3 BoneAnimationTrack::Interpolate(float animationTime) const
{
    if (m_PositionKeys.size() == 1)
    {
        return m_PositionKeys[0].Property;
    }
    else if (!m_PositionKeys.empty())
    {
        size_t positionIndex = GetIndex(animationTime, m_PositionKeys);
        size_t nextPositionIndex = positionIndex + 1;

        float deltaTime = m_PositionKeys[nextPositionIndex].Timestamp - m_PositionKeys[positionIndex].Timestamp;
        float factor = (animationTime - m_PositionKeys[positionIndex].Timestamp) / deltaTime;
        ASSERT(factor >= 0 && factor <= 1);

        return glm::mix(m_PositionKeys[positionIndex].Property, m_PositionKeys[nextPositionIndex].Property, factor);
    }
    else
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
    }
    else if (!m_RotationKeys.empty())
    {
        // find time range based on animationTime
        size_t rotationIndex = GetIndex(animationTime, m_RotationKeys);
        size_t nextRotationIndex = rotationIndex + 1;

        float deltaTime = m_RotationKeys[nextRotationIndex].Timestamp - m_RotationKeys[rotationIndex].Timestamp;
        float factor = (animationTime - m_RotationKeys[rotationIndex].Timestamp) / deltaTime;
        ASSERT(factor >= 0 && factor <= 1);
        return glm::mix(m_RotationKeys[rotationIndex].Property, m_RotationKeys[nextRotationIndex].Property, factor);
    }

    return glm::quat{glm::vec3{0, 0, 0}};
}


template <typename T>
inline size_t BoneAnimationTrack::GetIndex(float animationTime, const std::vector<KeyProperty<T>>& keys) const
{
    ASSERT(keys.size() > 0 && "Called BoneAnimationTrack::GetIndex with track without keys");

    // run binary search to find first timestamp that is greater than animationTime (max in time range)
    auto it = std::lower_bound(keys.begin(), keys.end(), animationTime, [](const KeyProperty<T>& k, float time)
    {
        return time > k.Timestamp;
    });

    bool bLastAnimTimestamp = it == keys.end();
    if (bLastAnimTimestamp)
    {
        return keys.size() > 2 ? static_cast<uint32_t>(keys.size() - 2) : 0;
    }

    size_t leftSideRange = static_cast<size_t>(std::distance(keys.begin(), it));

    if (leftSideRange != 0)
    {
        // if it's not a first frame of animation, adjust that i now defines left side of range
        // otherwise it's just range {0, 1}
        --leftSideRange;
    }

    return leftSideRange;
}

inline BoneInfo::BoneInfo(int boneTransformIndex, const glm::mat4& offsetMatrix) :
    BoneTransformIndex{boneTransformIndex},
    OffsetMatrix{offsetMatrix}
{
}

inline void SkeletalMesh::UpdateAnimation(const AnimationUpdateArgs& updateArgs) const
{
    const SkeletalAnimation& animation = m_Animations.at(updateArgs.AnimationName);

    // precalculate animation time to
    float timeInTicks = updateArgs.ElapsedTime * animation.TicksPerSecond;
    float animationTime = fmod(timeInTicks, animation.Duration);

    // run transform update chain starting from root joint
    CalculateTransform(BoneAnimationUpdateArgs{&animation, animationTime, &m_RootBone, updateArgs.Transforms});
}


inline void BoneAnimationTrack::AddNewPositionTimestamp(glm::vec3 position, float timestamp)
{
    m_PositionKeys.emplace_back(VectorProperty{position, timestamp});
}

inline void BoneAnimationTrack::AddNewRotationTimestamp(glm::quat Rotation, float timestamp)
{
    m_RotationKeys.emplace_back(QuatProperty{Rotation, timestamp});
}

FORCE_INLINE const SkeletalAnimation* BoneAnimationUpdateArgs::operator->() const
{
    ASSERT(UsedAnimation != nullptr);
    return UsedAnimation;
}

FORCE_INLINE void BoneAnimationUpdateArgs::UpdateTransformAt(int index, const glm::mat4& transform) const
{
    ASSERT(index < BoneTransforms.size());
    BoneTransforms[index] = transform;
}

FORCE_INLINE const glm::vec3& SkeletalMesh::GetBboxMin() const
{
    return m_BoundingBox.MinBounds;
}

FORCE_INLINE const glm::vec3& SkeletalMesh::GetBboxMax() const
{
    return m_BoundingBox.MaxBounds;
}

FORCE_INLINE const std::shared_ptr<Material>& SkeletalMesh::GetMaterial() const
{
    return MainMaterial;
}

FORCE_INLINE uint32_t SkeletalMesh::GetNumBones() const
{
    return m_NumBones;
}