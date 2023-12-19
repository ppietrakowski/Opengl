#include "MeshLoader.h"
#include "ObjMeshLoader.h"

#include "Logging.h"

#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <stdexcept>
#include <unordered_map>
#include <string>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>


StaticMeshImporter::StaticMeshImporter(std::filesystem::path path)
{
    if (path.has_extension() && path.extension() == ".obj")
    {
        _staticMeshLoader.reset(new StaticObjMeshLoader());
    }

    if (_staticMeshLoader)
    {
        this->_errorOccured = !_staticMeshLoader->Load(path.string());
        this->_errorMessage = _staticMeshLoader->GetLastErrorMessage();
    }
    else
    {
        _errorOccured = true;
        _errorMessage = "Unknown mesh format";
    }

    if (_errorOccured)
    {
        ELOG_ERROR(LOG_ASSET_LOADING, "%s", _errorMessage.c_str());
    }
}

std::span<const StaticMeshVertex> StaticMeshImporter::GetVertices() const
{
    ASSERT(!_errorOccured);
    return _staticMeshLoader->GetVertices();
}

std::span<const std::uint32_t> StaticMeshImporter::GetIndices() const
{
    ASSERT(!_errorOccured);
    return _staticMeshLoader->GetIndices();
}

std::string_view StaticMeshImporter::GetModelName() const
{
    ASSERT(!_errorOccured);
    return _staticMeshLoader->GetModelName();
}

std::string StaticMeshImporter::GetLastErrorMessage() const
{
    return _errorMessage;
}

std::runtime_error StaticMeshImporter::GetError() const
{
    return std::runtime_error(_errorMessage.c_str());
}

bool StaticMeshImporter::HasErrorOccured() const
{
    return _errorOccured;
}

std::uint32_t StaticMeshImporter::GetNumIndices() const
{
    ASSERT(!_errorOccured);
    return static_cast<std::uint32_t>(GetIndices().size());
}

std::uint32_t StaticMeshImporter::GetNumVertices() const
{
    ASSERT(!_errorOccured);
    return static_cast<std::uint32_t>(GetVertices().size());
}

inline glm::mat4 ToGlm(aiMatrix4x4 mat)
{
    return glm::make_mat4(mat.Transpose()[0]);
}

inline glm::vec3 ToGlm(aiVector3D v)
{
    return glm::vec3{ v.x, v.y, v.z };
}

inline glm::vec2 ToGlm(aiVector2D v)
{
    return glm::vec2{ v.x, v.y };
}

inline glm::quat ToGlm(aiQuaternion v)
{
    return glm::quat{ v.w, v.x, v.y, v.z };
}

SkeletonMeshImporter::SkeletonMeshImporter(std::filesystem::path path)
{
}

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
