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

inline constexpr std::int32_t kNumBonesPerVertex = 4;
static constexpr const char* kDefaultAnimationName = "TPose";

struct SkeletonMeshVertex
{
    glm::vec3 position{0, 0,0};
    glm::vec3 normal{0, 0, 0};
    glm::vec2 texture_coords{0, 0};
    std::int32_t bone_ids[kNumBonesPerVertex] = {0, 0, 0, 0};
    float bone_weights[kNumBonesPerVertex] = {0, 0, 0, 0};
    std::uint32_t texture_id{0};

    static inline constexpr VertexAttribute kDataFormat[6] = {
        { 3, PrimitiveVertexType::kFloat },
        { 3, PrimitiveVertexType::kFloat },
        { 2, PrimitiveVertexType::kFloat },
        { kNumBonesPerVertex, PrimitiveVertexType::kInt },
        { kNumBonesPerVertex, PrimitiveVertexType::kFloat },
        { 1, PrimitiveVertexType::kUnsignedInt }
    };

    SkeletonMeshVertex() = default;
    SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture_coords);
    SkeletonMeshVertex(const SkeletonMeshVertex&) = default;
    SkeletonMeshVertex& operator=(const SkeletonMeshVertex&) = default;

    bool AddBoneData(std::int32_t bone_id, float weight);
};

template <typename T>
struct KeyProperty
{
    T property;
    float timestamp;
};

using VectorProperty = KeyProperty<glm::vec3>;
using QuatProperty = KeyProperty<glm::quat>;

class BoneAnimationTrack
{
public:
    void AddNewPositionTimestamp(glm::vec3 position, float timestamp);
    void AddNewRotationTimestamp(glm::quat rotation, float timestamp);

    template <typename T>
    T Interpolate(float animation_time) const;

    template<>
    glm::vec3 Interpolate<glm::vec3>(float animation_time) const;

    template<>
    glm::quat Interpolate<glm::quat>(float animation_time) const;

private:
    std::vector<VectorProperty> position_keys_;
    std::vector<QuatProperty> rotation_keys_;

    template <typename T>
    std::int32_t GetIndex(float animation_time, const std::vector<KeyProperty<T>>& timestamps) const;
};


struct BoneInfo
{
    std::int32_t bone_transform_index;

    /* Matrix that convert vertex to bone space */
    glm::mat4 offset_matrix;

    BoneInfo() = default;
    BoneInfo(const BoneInfo&) = default;
    BoneInfo& operator=(const BoneInfo&) = default;
    BoneInfo(std::int32_t bone_transform_index, const glm::mat4& offset_matrix) :
        bone_transform_index{bone_transform_index},
        offset_matrix{offset_matrix}
    {
    }
};

struct aiNode;

struct Bone
{
    std::vector<Bone> children;

    /* Index in bone_transform_ array */
    std::int32_t bone_transform_index{0};

    /* Relative transformation to it's parent */
    glm::mat4 relative_transform_matrix{glm::identity<glm::mat4>()};

    /* Matrix that convert vertex to bone space */
    glm::mat4 bone_offset{glm::identity<glm::mat4>()};
    std::string name;

    bool AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& bones_info);
};

struct SkeletalAnimation
{
    // Duration in ticks
    float duration{0.0f};
    float ticks_per_second{0.0f};

    // bone name mapped to animation track
    std::unordered_map<std::string, BoneAnimationTrack> bone_name_to_tracks;

    glm::mat4 GetBoneTransformOrRelative(const Bone& bone, float animation_time) const;
};

struct aiScene;

struct BoneAnimationUpdateSpecs
{
    const SkeletalAnimation* animation;
    float animation_time;
    const Bone* target_bone;

    const SkeletalAnimation* operator->() const
    {
        ASSERT(animation != nullptr);
        return animation;
    }
};

class SkeletalMesh
{
    friend struct SkeletalMeshComponent;

public:
    SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material);

    std::vector<std::string> GetAnimationNames() const;

    void GetAnimationFrames(float elapsed_time, const std::string& name, std::vector<glm::mat4>& transforms) const;

    const glm::vec3& GetBboxMin() const
    {
        return bbox_min_;
    }

    const glm::vec3& GetBboxMax() const
    {
        return bbox_max_;
    }

    const std::shared_ptr<Material>& GetMaterial() const
    {
        return main_material;
    }

    std::int32_t GetNumBones() const
    {
        return num_bones_;
    }

    std::shared_ptr<Material> main_material;
    std::vector<std::string> texture_paths;

private:
    std::shared_ptr<VertexArray> vertex_array_;
    Bone root_bone_;
    std::unordered_map<std::string, SkeletalAnimation> animations_;
    glm::mat4 global_inverse_transform_;

    std::int32_t num_bones_;

    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;

private:
    void UpdateAnimation(const std::string& animation_name, float elapsed_time, std::vector<glm::mat4>& transforms) const;
    void CalculateTransform(const BoneAnimationUpdateSpecs& update_specs, std::vector<glm::mat4>& transforms, const glm::mat4& parent_transform = glm::identity<glm::mat4>()) const;
    std::shared_ptr<Texture2D> LoadTexturesFromMaterial(const aiScene* scene, std::int32_t material_index);
    void LoadAnimation(const aiScene* scene, std::int32_t animation_index);

    void Draw(const std::vector<glm::mat4>& transforms, const glm::mat4& world_transform);
};

template<>
inline glm::vec3 BoneAnimationTrack::Interpolate(float animation_time) const
{
    if (position_keys_.size() == 1)
    {
        return position_keys_[0].property;
    } else if (!position_keys_.empty())
    {
        std::int32_t pos_index = GetIndex(animation_time, position_keys_);
        std::int32_t next_pos_index = pos_index + 1;

        float delta_time = position_keys_[next_pos_index].timestamp - position_keys_[pos_index].timestamp;
        float factor = (animation_time - position_keys_[pos_index].timestamp) / delta_time;
        ASSERT(factor >= 0 && factor <= 1);

        return glm::mix(position_keys_[pos_index].property, position_keys_[next_pos_index].property, factor);
    } else
    {
        return glm::vec3{0, 0, 0};
    }
}

template<>
inline glm::quat BoneAnimationTrack::Interpolate(float animation_time) const
{
    if (rotation_keys_.size() == 1)
    {
        // not enough keys, use first key as base
        return rotation_keys_[0].property;
    } else if (!rotation_keys_.empty())
    {
        // find time range based on animation_time
        std::int32_t rot_index = GetIndex(animation_time, rotation_keys_);
        std::int32_t next_rot_index = rot_index + 1;

        float delta_time = rotation_keys_[next_rot_index].timestamp - rotation_keys_[rot_index].timestamp;
        float factor = (animation_time - rotation_keys_[rot_index].timestamp) / delta_time;
        ASSERT(factor >= 0 && factor <= 1);
        return glm::mix(rotation_keys_[rot_index].property, rotation_keys_[next_rot_index].property, factor);
    }

    return glm::quat{glm::vec3{0, 0, 0}};
}


template <typename T>
inline std::int32_t BoneAnimationTrack::GetIndex(float animation_time, const std::vector<KeyProperty<T>>& keys) const
{
    ASSERT(keys.size() > 0 && "Called BoneAnimationTrack::GetIndex with track without keys");

    // run binary search to find first timestamp that is greater than animation_time (max in time range)
    auto it = std::lower_bound(keys.begin(), keys.end(), animation_time, [](const KeyProperty<T>& k, float time) {
        return time > k.timestamp;
    });

    bool last_anim_timestamp = it == keys.end();
    if (last_anim_timestamp)
    {
        return keys.size() > 2 ? static_cast<uint32_t>(keys.size() - 2) : 0;
    }

    std::int32_t left_side_range = static_cast<std::int32_t>(std::distance(keys.begin(), it));
    ASSERT(left_side_range >= 0);

    if (left_side_range != 0)
    {
        // if it's not a first frame of animation, adjust that i now defines left side of range
        // otherwise it's just range {0, 1}
        --left_side_range;
    }

    return left_side_range;
}

inline void SkeletalMesh::UpdateAnimation(const std::string& animation_name, float elapsed_time, std::vector<glm::mat4>& transforms) const
{
    const SkeletalAnimation& animation = animations_.at(animation_name);

    // precalculate animation time to
    float time_in_ticks = elapsed_time * animation.ticks_per_second;
    float animation_time = fmod(time_in_ticks, animation.duration);

    // run transform update chain starting from root joint
    CalculateTransform(BoneAnimationUpdateSpecs{&animation, animation_time, &root_bone_}, transforms);
}


inline void BoneAnimationTrack::AddNewPositionTimestamp(glm::vec3 position, float timestamp)
{
    position_keys_.emplace_back(VectorProperty{position, timestamp});
}

inline void BoneAnimationTrack::AddNewRotationTimestamp(glm::quat Rotation, float timestamp)
{
    rotation_keys_.emplace_back(QuatProperty{Rotation, timestamp});
}