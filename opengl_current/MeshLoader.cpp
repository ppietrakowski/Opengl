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

bool readSkeleton(Joint& boneOutput, aiNode* node, std::unordered_map<std::string, std::pair<std::uint32_t, glm::mat4>>& boneInfoTable)
{
    auto it = boneInfoTable.find(node->mName.C_Str());

    if (it != boneInfoTable.end())
    { // if node is actually a bone
        boneOutput.Name = node->mName.C_Str();
        auto& boneInfo = it->second;

        boneOutput.IndexInBoneTransformArray = boneInfo.first;
        boneOutput.RelativeTransformMatrix = boneInfo.second;

        for (std::uint32_t i = 0; i < node->mNumChildren; i++)
        {
            Joint child;
            readSkeleton(child, node->mChildren[i], boneInfoTable);
            boneOutput.Children.emplace_back(child);
        }
        return true;
    }
    else
    { // find bones in children
        for (std::uint32_t i = 0; i < node->mNumChildren; i++)
        {
            if (readSkeleton(boneOutput, node->mChildren[i], boneInfoTable))
            {
                return true;
            }
        }
    }
    return false;
}

SkeletonMeshImporter::SkeletonMeshImporter(std::filesystem::path path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs);

    if (scene == nullptr)
    {
        throw importer.GetException();
    }

    std::vector<BonesInfluencingVertex> vertexToBones;
    std::unordered_map<std::string, std::uint32_t> boneNameToIndex;

    auto getBoneID = [&](const aiBone* bone)
    {
        std::uint32_t boneID;
        std::string boneName(bone->mName.C_Str());

        auto it = boneNameToIndex.find(boneName);

        if (it != boneNameToIndex.end())
        {
            boneID = it->second;
        }
        else
        {
            boneID = boneNameToIndex.size();
            boneNameToIndex[boneName] = boneID;
        }

        return boneID;
    };

    const aiMesh* mesh = scene->mMeshes[0];

    vertexToBones.resize(mesh->mNumVertices);
    _vertices.reserve(mesh->mNumVertices);

    for (std::uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        _vertices.emplace_back(ToGlm(mesh->mVertices[i]), ToGlm(mesh->mNormals[i]), ToGlm(mesh->mTextureCoords[0][i]));
    }

    std::vector<std::uint32_t> boneCounts(_vertices.size(), 0);
    std::unordered_map<std::string, std::pair<std::uint32_t, glm::mat4>> boneInfo;

    

    for (int i = 0; i < _vertices.size(); i++)
    {
        auto& boneWeights = _vertices[i].Weights;
        float totalWeight = boneWeights[0] + boneWeights[1] + boneWeights[2] + boneWeights[3];
        if (totalWeight > 0.0f)
        {
            for (size_t j = 0; j < boneWeights.size(); ++j)
            {
                boneWeights[j] = boneWeights[j] / totalWeight;
            }
        }

    }
    for (std::uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        for (std::uint32_t j = 0; j < face.mNumIndices; j++)
            _indices.emplace_back(face.mIndices[j]);
    }

    readSkeleton(_rootJoint, scene->mRootNode, boneInfo);

    for (std::uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* anim = scene->mAnimations[i];

        Animation animation{};
        if (anim->mTicksPerSecond != 0.0f)
            animation.TicksPerSecond = anim->mTicksPerSecond;
        else
            animation.TicksPerSecond = 1;


        animation.Duration = anim->mDuration * anim->mTicksPerSecond;

        for (std::uint32_t i = 0; i < anim->mNumChannels; i++)
        {
            aiNodeAnim* channel = anim->mChannels[i];
            BoneAnimationTrack track;
            for (std::uint32_t j = 0; j < channel->mNumPositionKeys; j++)
            {
                track.AddNewPositionTimestamp(ToGlm(channel->mPositionKeys[j].mValue), channel->mPositionKeys[j].mTime);
            }
            for (std::uint32_t j = 0; j < channel->mNumRotationKeys; j++)
            {
                track.AddNewRotationTimestamp(ToGlm(channel->mRotationKeys[j].mValue), channel->mRotationKeys[j].mTime);
            }

            animation.BoneNamesToTracks[channel->mNodeName.C_Str()] = track;
        }
        _animations[anim->mName.C_Str()] = animation;
    }

    _boneCount = mesh->mNumBones;
    GlobalInversedTransform = glm::inverse(ToGlm(scene->mRootNode->mTransformation));
    std::int32_t j{ 0 };
    std::cout << j << std::endl;
}

void BonesInfluencingVertex::AddNewBone(std::uint32_t boneID, float weight)
{
    auto it = std::find(Weights.begin(), Weights.end(), 0.0f);

    if (it != Weights.end())
    {
        std::size_t index = std::distance(Weights.begin(), it);

        Weights[index] = weight;
        BoneIDs[index] = boneID;
    }
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
