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

#include <chrono>

inline constexpr std::size_t kNumBonesPerVertex = 4;

struct SkeletonMeshVertex {
    glm::vec3 position{ 0, 0,0 };
    glm::vec3 normal{ 0, 0, 0 };
    glm::vec2 texture_coords{ 0, 0 };
    float bone_ids[kNumBonesPerVertex] = { 0, 0, 0, 0 };
    float bone_weights[kNumBonesPerVertex] = { 0, 0, 0, 0 };
    std::uint32_t texture_id{ 0 };

    static inline constexpr VertexAttribute data_format[6] = {
        { 3, PrimitiveVertexType::kFloat },
        { 3, PrimitiveVertexType::kFloat },
        { 2, PrimitiveVertexType::kFloat },
        { kNumBonesPerVertex, PrimitiveVertexType::kFloat },
        { kNumBonesPerVertex, PrimitiveVertexType::kFloat },
        { 1, PrimitiveVertexType::kUnsignedInt }
    };

    SkeletonMeshVertex() = default;
    SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture_coords);
    SkeletonMeshVertex(const SkeletonMeshVertex&) = default;
    SkeletonMeshVertex& operator=(const SkeletonMeshVertex&) = default;
    bool AddBoneData(std::uint32_t bone_id, float weight);
};

template <typename T>
struct KeyProperty {
    T property;
    float timestamp;
};

class BoneAnimationTrack {
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
    std::vector<KeyProperty<glm::vec3>> positions_;
    std::vector<KeyProperty<glm::quat>> rotations_;

    template <typename T>
    std::uint32_t GetIndex(float animation_time, const std::vector<KeyProperty<T>>& timestamps) const;
};

struct Animation {
    // Duration in ticks
    float duration{ 0.0f };
    float ticks_per_second{ 0.0f };

    // bone name mapped to animation track
    std::unordered_map<std::string, BoneAnimationTrack> bone_name_to_tracks;

    glm::mat4 GetBoneTransformOrRelative(const std::string& bone_name, float animation_time, glm::mat4 relative_transform) const;
};

struct BoneInfo {
    std::uint32_t bone_transform_index;

    /* Matrix that convert vertex to bone space */
    glm::mat4 offset_matrix;

    BoneInfo() = default;
    BoneInfo(const BoneInfo&) = default;
    BoneInfo& operator=(const BoneInfo&) = default;
    BoneInfo(std::uint32_t bone_transform_index, const glm::mat4& offset_matrix) :
        bone_transform_index{ bone_transform_index },
        offset_matrix{ offset_matrix } {}
};

struct aiNode;

struct Joint {
    std::vector<Joint> children;

    /* Index in bone_transform_ array */
    std::uint32_t bone_transform_index{ 0 };

    /* Relative transformation to it's parent */
    glm::mat4 relative_transform_matrix{ glm::identity<glm::mat4>() };

    /* Matrix that convert vertex to bone space */
    glm::mat4 bone_offset{ glm::identity<glm::mat4>() };
    std::string name;

    bool AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& bones_info);
};


struct aiScene;

class SkeletalMesh {
public:
    SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material);

    void UpdateAnimation(float elapsed_time);

    void Draw(const glm::mat4& transform);

    void SetCurrentAnimation(const std::string& animation_name);
    std::vector<std::string> GetAnimationNames() const;

private:
    std::vector<std::shared_ptr<Texture2D>> textures_;
    VertexArray vertex_array_;
    std::vector<glm::mat4> bone_transforms_;
    std::vector<SkeletonMeshVertex> vertices_;
    std::vector<std::uint32_t> indices_;


    Joint root_joint_;
    std::unordered_map<std::string, Animation> animations_;
    glm::mat4 global_inverse_transform_;

    std::uint32_t num_bones_;

    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;
    std::shared_ptr<Material> material_;

    std::string current_animation_name_;

private:
    void CalculateTransform(float animation_time, const Joint& joint, const glm::mat4& parent_transform = glm::mat4{ 1.0f });
    void LoadTexturesFromMaterial(const aiScene* scene, uint32_t material_index);
    void LoadAnimation(const aiScene* scene, uint32_t animation_index);
};

template<>
inline glm::vec3 BoneAnimationTrack::Interpolate(float animation_time) const {
    if (positions_.size() == 1) {
        return positions_[0].property;
    }

    else if (!positions_.empty()) {
        std::uint32_t position_index = GetIndex(animation_time, positions_);
        std::uint32_t next_position_index = position_index + 1;

        float delta_time = positions_[next_position_index].timestamp - positions_[position_index].timestamp;
        float factor = (animation_time - positions_[position_index].timestamp) / delta_time;
        ASSERT(factor >= 0 && factor <= 1);

        return glm::mix(positions_[position_index].property, positions_[next_position_index].property, factor);
    } else {
        return glm::vec3{ 0, 0, 0 };
    }
}

template<>
inline glm::quat BoneAnimationTrack::Interpolate(float animation_time) const {
    if (rotations_.size() == 1) {
        return rotations_[0].property;
    }

    else if (!rotations_.empty()) {
        std::uint32_t rotation_index = GetIndex(animation_time, rotations_);
        std::uint32_t next_rotation_index = rotation_index + 1;

        float delta_time = rotations_[next_rotation_index].timestamp - rotations_[rotation_index].timestamp;
        float factor = (animation_time - rotations_[rotation_index].timestamp) / delta_time;
        ASSERT(factor >= 0 && factor <= 1);
        return glm::mix(rotations_[rotation_index].property, rotations_[next_rotation_index].property, factor);
    }

    return glm::quat{ glm::vec3{0, 0, 0} };
}


template <typename T>
inline std::uint32_t BoneAnimationTrack::GetIndex(float animation_time, const std::vector<KeyProperty<T>>& keys) const {
    ASSERT(keys.size() > 0);

    for (std::uint32_t i = 0; i < keys.size() - 1; i++) {
        float t = keys[i + 1].timestamp;
        if (animation_time < t) {
            return i;
        }
    }

    return 0;
}

inline void SkeletalMesh::UpdateAnimation(float elapsed_time) {
    const Animation& animation = animations_.at(current_animation_name_);
    float ticks_per_second = animation.ticks_per_second;
    float time_in_ticks = elapsed_time * ticks_per_second;
    float animation_time = fmod(time_in_ticks, animation.duration);

    // run transform update chain starting from root joint
    CalculateTransform(animation_time, root_joint_);
}