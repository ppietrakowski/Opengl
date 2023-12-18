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

struct StaticMeshVertex
{
    glm::vec3 Position{ 0, 0,0 };
    glm::vec3 Normal{ 0, 0, 0 };
    glm::vec2 TextureCoords{ 0, 0 };


    static inline constexpr VertexAttribute DataFormat[3] = { {3, PrimitiveVertexType::Float}, {3, PrimitiveVertexType::Float}, {2, PrimitiveVertexType::Float} };

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

struct BonesInfluencingVertex
{
    std::array<std::uint32_t, NumBonesPerVertex> BoneIDs{ 0 };
    std::array<float, NumBonesPerVertex> Weights{ 0.0f };

    void AddNewBone(std::uint32_t boneID, float weight);
};

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
        {3, PrimitiveVertexType::UnsignedInt},
        {3, PrimitiveVertexType::Float}
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

struct BoneAnimation
{
    glm::vec3 Offset;
    glm::quat Rotation;
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
};

struct Animation
{
    float Duration{ 0.0f };
    float TicksPerSecond{ 0.0f };
    std::unordered_map<std::string, BoneAnimationTrack> BoneNamesToTracks;
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

struct Joint
{
    std::vector<Joint> Children;
    std::uint32_t IndexInBoneTransformArray{ 0 };
    glm::mat4 OffsetMatrix{ glm::identity<glm::mat4>()};
    std::string Name;
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