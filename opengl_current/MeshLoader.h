#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>
#include <span>

#include <string>
#include <memory>

#include <cstdint>
#include <filesystem>
#include <array>

#include <glm/gtc/quaternion.hpp>
#include <unordered_map>
#include "ErrorMacros.h"

struct StaticMeshVertex
{
    glm::vec3 Position{ 0, 0,0 };
    glm::vec3 Normal{ 0, 0, 0 };
    glm::vec2 TextureCoords{ 0, 0 };
    std::uint32_t ID{ 0 };

    static inline constexpr VertexAttribute DataFormat[4] = { {3, PrimitiveVertexType::Float}, {3, PrimitiveVertexType::Float}, {2, PrimitiveVertexType::Float}, {1, PrimitiveVertexType::UnsignedInt} };

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
        Position{ position },
        Normal{ normal },
        TextureCoords{ textureCoords }
    {
    }

    StaticMeshVertex(const StaticMeshVertex&) = default;
    StaticMeshVertex& operator=(const StaticMeshVertex&) = default;
};

inline constexpr std::size_t NumBonesPerVertex = 4;

struct SkeletonMeshVertex
{
    glm::vec3 Position{ 0, 0,0 };
    glm::vec3 Normal{ 0, 0, 0 };
    glm::vec2 TextureCoords{ 0, 0 };

    std::array<std::uint32_t, NumBonesPerVertex> BoneIds{ 0, 0, 0, 0 };
    std::array<float, NumBonesPerVertex> Weights{ 0, 0, 0, 0 };

    static inline constexpr VertexAttribute DataFormat[5] = {
        {3, PrimitiveVertexType::Float},
        {3, PrimitiveVertexType::Float},
        {2, PrimitiveVertexType::Float},
        {4, PrimitiveVertexType::UnsignedInt},
        {4, PrimitiveVertexType::Float}
    };

    SkeletonMeshVertex() = default;
    SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
        Position{ position },
        Normal{ normal },
        TextureCoords{ textureCoords }
    {
    }

    SkeletonMeshVertex(const SkeletonMeshVertex&) = default;
    SkeletonMeshVertex& operator=(const SkeletonMeshVertex&) = default;
};

struct BoneAnimationTrack
{
    std::vector<float> PositionTimestamps;
    std::vector<float> RotationsTimestamps;

    // Positions in specific timestamps (size same as PositionTimestamps)
    std::vector<glm::vec3> Positions;

    // Rotations in specific timestamps (size same as RotationsTimestamps)
    std::vector<glm::quat> Rotations;

    void AddNewPositionTimestamp(glm::vec3 Position, float Timestamp);
    void AddNewRotationTimestamp(glm::quat Rotation, float Timestamp);

    template <typename T>
    T Interpolate(float animationTime) const;

    template<>
    glm::vec3 Interpolate<glm::vec3>(float animationTime) const;

    template<>
    glm::quat Interpolate<glm::quat>(float animationTime) const;

    std::uint32_t GetIndex(float animationTime, const std::vector<float>& timestamps) const;
};

struct Animation
{
    float Duration{ 0.0f };
    float TicksPerSecond{ 0.0f };
    std::unordered_map<std::string, BoneAnimationTrack> BoneNamesToTracks;

    glm::mat4 GetBoneTransformOrRelative(const std::string& boneName, float animationTime, glm::mat4 relativeTransform) const;
    glm::mat4 GetBoneTransform(const std::string& boneName, float animationTime) const;
};

class IStaticMeshLoader
{
public:
    virtual ~IStaticMeshLoader() = default;

    /*
    * Loads mesh from specified path
    * Should return true, if no error occured during loading
    */
    virtual bool Load(const std::string& path) = 0;

    virtual std::span<const StaticMeshVertex> GetVertices() const = 0;
    virtual std::span<const std::uint32_t> GetIndices() const = 0;
    virtual std::string_view GetModelName() const = 0;

    virtual std::string GetLastErrorMessage() const = 0;
};

class StaticMeshImporter
{
public:
    StaticMeshImporter(std::filesystem::path path);

    std::span<const StaticMeshVertex> GetVertices() const;
    std::span<const std::uint32_t> GetIndices() const;
    std::string_view GetModelName() const;

    std::string GetLastErrorMessage() const;

    std::runtime_error GetError() const;

    bool HasErrorOccured() const;

    std::uint32_t GetNumIndices() const;
    std::uint32_t GetNumVertices() const;

private:
    std::unique_ptr<IStaticMeshLoader> _staticMeshLoader;
    std::string _errorMessage;
    bool _errorOccured : 1;
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


class SkeletonMeshImporter
{
public:
    SkeletonMeshImporter(std::filesystem::path path);

public:
    std::span<const SkeletonMeshVertex> GetVertices() const;
    std::span<const std::uint32_t> GetIndices() const;
    std::string_view GetModelName() const;

    std::string GetLastErrorMessage() const;
    std::runtime_error GetError() const;

    bool HasErrorOccured() const;

    std::uint32_t GetNumIndices() const;
    std::uint32_t GetNumVertices() const;

public:
    std::vector<SkeletonMeshVertex> _vertices;
    std::vector<std::uint32_t> _indices;
    std::string _name;
    std::uint32_t _boneCount;
    Joint _rootJoint;
    std::unordered_map<std::string, Animation> _animations;
    glm::mat4 GlobalInversedTransform;
};


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