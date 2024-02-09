#include "SkeletalMesh.hpp"
#include "AssimpUtils.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Core.hpp"
#include "ResourceManager.hpp"
#include "Logging.hpp"

static void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax);

glm::mat4 SkeletalAnimation::GetBoneTransformOrRelative(const Bone& bone, float animationTime) const
{
    auto it = BoneNameToTracks.find(bone.Name);

    if (it == BoneNameToTracks.end())
    {
        // track for this bone could not be found, so take default relative_transform_matrix
        return bone.RelativeTransformMatrix;
    }

    const BoneAnimationTrack& track = it->second;
    glm::vec3 position = track.Interpolate<glm::vec3>(animationTime);
    glm::quat rotation = track.Interpolate<glm::quat>(animationTime);
    return glm::translate(position) * glm::mat4_cast(rotation);
}

bool Bone::AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& bonesInfo)
{
    auto it = bonesInfo.find(node->mName.C_Str());
    bool is_bone = it != bonesInfo.end();

    if (is_bone)
    {
        Name = node->mName.C_Str();
        const BoneInfo& boneInfo = it->second;

        BoneTransformIndex = boneInfo.BoneTransformIndex;
        RelativeTransformMatrix = ToGlm(node->mTransformation);
        BoneOffset = boneInfo.OffsetMatrix;

        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            Bone child;
            child.AssignHierarchy(node->mChildren[i], bonesInfo);
            Children.emplace_back(child);
        }

        return true;
    }
    else
    {
        // traverse all children to find bone
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            if (AssignHierarchy(node->mChildren[i], bonesInfo))
            {
                return true;
            }
        }
    }

    // traversed all children and it's not a bone
    return false;
}

SkeletonMeshVertex::SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
    Position{position},
    Normal{normal},
    TextureCoords{textureCoords}
{
}

bool SkeletonMeshVertex::AddBoneData(int boneId, float weight)
{
    // find first empty slot
    auto it = std::find(std::begin(BoneWeights), std::end(BoneWeights), 0.0f);

    if (it != std::end(BoneWeights))
    {
        ptrdiff_t index = std::distance(std::begin(BoneWeights), it);
        BoneIds[index] = boneId;
        BoneWeights[index] = weight;

        return true;
    }

    return false;
}

SkeletalMesh::SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material) :
    MainMaterial{material},
    m_NumBones{0},
    m_VertexArray{std::make_shared<VertexArray>()}
{
    // maps bone name to boneID
    std::unordered_map<std::string, int> boneNameToIndex;

    auto getBoneId = [&boneNameToIndex](const aiBone* bone)
    {
        int boneId = 0;
        std::string boneName(bone->mName.C_Str());

        auto it = boneNameToIndex.find(boneName);

        bool bBoneIdAlreadyDefined = it != boneNameToIndex.end();

        if (bBoneIdAlreadyDefined)
        {
            boneId = it->second;
        }
        else
        {
            boneId = static_cast<int>(boneNameToIndex.size());
            boneNameToIndex[boneName] = boneId;
        }

        return boneId;
    };

    int totalVertices = 0;
    int totalIndices = 0;

    Assimp::Importer importer;

    std::string filePath = path.string();
    const aiScene* scene = importer.ReadFile(filePath, AssimpImportFlags);
    ENG_LOG_VERBOSE("Loading {} skeletal mesh", filePath);
    CRASH_EXPECTED_NOT_NULL(scene);

    // used for reserve enough indices to decrease allocating overhead
    int startNumIndices = scene->mMeshes[0]->mNumFaces * 3;

    // packed all vertices of all meshes in aiScene
    std::vector<SkeletonMeshVertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(scene->mMeshes[0]->mNumVertices);
    indices.reserve(startNumIndices);

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        LoadTexturesFromMaterial(scene, i);
    }

    std::unordered_map<std::string, BoneInfo> bonesInfo;

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j)
        {
            const aiVector3D& pos = mesh->mVertices[j];
            const aiVector3D& normal = mesh->mNormals[j];
            const aiVector3D& textureCoords = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(textureCoords));
            vertices.back().TextureId = mesh->mMaterialIndex;
        }

        for (uint32_t j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3 && "Face is not triangulated");

            for (uint32_t k = 0; k < face.mNumIndices; ++k)
            {
                indices.emplace_back(face.mIndices[k] + totalIndices);
            }
        }

        for (uint32_t bone_index = 0; bone_index < mesh->mNumBones; ++bone_index)
        {
            const aiBone* bone = mesh->mBones[bone_index];
            int boneId = getBoneId(bone);

            glm::mat4 offsetMatrix = ToGlm(bone->mOffsetMatrix);
            bonesInfo[bone->mName.C_Str()] = BoneInfo{static_cast<int>(bone_index), offsetMatrix};
            std::string s{bone->mName.C_Str()};

            for (uint32_t j = 0; j < bone->mNumWeights; j++)
            {
                // find global id of vertex
                uint32_t id = bone->mWeights[j].mVertexId + totalVertices;
                float weight = bone->mWeights[j].mWeight;

                if (!vertices[id].AddBoneData(boneId, weight))
                {
                    break;
                }
            }
        }

        totalVertices += mesh->mNumVertices;
        totalIndices += mesh->mNumFaces * 3;
        m_NumBones += mesh->mNumBones;
    }

    for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        LoadAnimation(scene, i);
    }

    // define Tpose animation dummy values
    m_Animations[DefaultAnimationName] = SkeletalAnimation{};
    m_Animations[DefaultAnimationName].TicksPerSecond = 30;
    m_Animations[DefaultAnimationName].Duration = 10;

    ENG_LOG_VERBOSE("Loaded skeletal mesh {0}, got anims: ", filePath);

    for (auto& [name, animation] : m_Animations)
    {
        ENG_LOG_VERBOSE("Animation {} [Duration: {}, TicksPerSecond: {}, NumTracks: {}]", name.c_str(), animation.Duration, animation.TicksPerSecond,
            (int)animation.BoneNameToTracks.size());
    }

    m_RootBone.AssignHierarchy(scene->mRootNode, bonesInfo);
    m_VertexArray->AddVertexBuffer(std::make_shared<VertexBuffer>(vertices.data(), static_cast<int>(vertices.size() * sizeof(SkeletonMeshVertex))), SkeletonMeshVertex::DataFormat);
    m_VertexArray->SetIndexBuffer(std::make_shared<IndexBuffer>(indices));

    // find global transform for converting from bone space back to local space
    m_GlobalInverseTransform = glm::inverse(ToGlm(scene->mRootNode->mTransformation));

    FindAabCollision(vertices, m_BoundingBox.MinBounds, m_BoundingBox.MaxBounds);
    m_BoundingBox.MinBounds *= 0.01f;
    m_BoundingBox.MaxBounds *= 0.01f;

    m_BoundingBox.MinBounds.x *= 0.2f;
    m_BoundingBox.MaxBounds.x *= 0.2f;
}

void SkeletalMesh::LoadAnimation(const aiScene* scene, int animationIndex)
{
    const aiAnimation* anim = scene->mAnimations[animationIndex];
    SkeletalAnimation animation{};

    if (anim->mTicksPerSecond != 0.0f)
    {
        animation.TicksPerSecond = static_cast<float>(anim->mTicksPerSecond);
    }
    else
    {
        animation.TicksPerSecond = 1.0f;
    }

    animation.Duration = static_cast<float>(anim->mDuration);

    for (uint32_t i = 0; i < anim->mNumChannels; i++)
    {
        const aiNodeAnim* channel = anim->mChannels[i];
        BoneAnimationTrack track;

        for (uint32_t j = 0; j < channel->mNumPositionKeys; j++)
        {
            track.AddNewPositionTimestamp(ToGlm(channel->mPositionKeys[j].mValue),
                static_cast<float>(channel->mPositionKeys[j].mTime));
        }
        for (uint32_t j = 0; j < channel->mNumRotationKeys; j++)
        {
            track.AddNewRotationTimestamp(ToGlm(channel->mRotationKeys[j].mValue),
                static_cast<float>(channel->mRotationKeys[j].mTime));
        }

        // skip scale tracks, as it's not common to use scaling tracks of bones

        std::string name = channel->mNodeName.C_Str();
        animation.BoneNameToTracks[name] = track;
    }

    std::string animationName = anim->mName.C_Str();
    animationName = SplitString(animationName, "|").back();
    m_Animations[animationName] = animation;
}

std::vector<std::string> SkeletalMesh::GetAnimationNames() const
{
    std::vector<std::string> names;

    names.reserve(m_Animations.size());

    for (auto& [name, animation] : m_Animations)
    {
        names.emplace_back(name);
    }

    return names;
}

void SkeletalMesh::GetAnimationFrames(const AnimationUpdateArgs& updateArgs) const
{
    ASSERT(updateArgs.Transforms.size() >= m_NumBones);
    UpdateAnimation(updateArgs);
}

void SkeletalMesh::CalculateTransform(const BoneAnimationUpdateArgs& updateArgs) const
{
    const Bone& bone = *updateArgs.TargetBone;
    glm::mat4 transform = updateArgs->GetBoneTransformOrRelative(bone, updateArgs.AnimationTime);

    int index = bone.BoneTransformIndex;
    glm::mat4 globalTransform = updateArgs.ParentTransform * transform;
    updateArgs.UpdateTransformAt(index, globalTransform * bone.BoneOffset);

    // run chain to update other joint transforms
    for (const Bone& child : bone.Children)
    {
        BoneAnimationUpdateArgs newUpdateArgs = updateArgs;
        newUpdateArgs.TargetBone = &child;
        newUpdateArgs.ParentTransform = globalTransform;

        CalculateTransform(newUpdateArgs);
    }
}

std::shared_ptr<Texture2D> SkeletalMesh::LoadTexturesFromMaterial(const aiScene* scene, int materialIndex)
{
    aiString texturePaths;

    if (scene->mMaterials[materialIndex]->GetTexture(aiTextureType_DIFFUSE, 0,
        &texturePaths, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS)
    {
        const aiTexture* texture = scene->GetEmbeddedTexture(texturePaths.C_Str());

        if (texture != nullptr)
        {
            bool bCompressed = texture->mHeight == 0;

            if (bCompressed)
            {
                ResourceManager::AddTexture2D(texturePaths.C_Str(),
                    std::make_shared<Texture2D>(LoadRgbaImageFromMemory(texture->pcData, texture->mWidth)));
            }
            else
            {
                ResourceManager::AddTexture2D(texturePaths.C_Str(), std::make_shared<Texture2D>(
                    LoadRgbaImageFromMemory(texture->pcData, texture->mWidth * texture->mHeight)));
            }

            TextureNames.emplace_back(texturePaths.C_Str());

            return ResourceManager::GetTexture2D(texturePaths.C_Str());
        }
    }

    return nullptr;
}

void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax)
{
    // assume mesh has infinite bounds
    outBoxMin = glm::vec3{std::numeric_limits<float>::max()};
    outBoxMax = glm::vec3{std::numeric_limits<float>::min()};

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const glm::vec3* vertex = &vertices[i].Position;

        if (vertex->x < outBoxMin.x)
        {
            outBoxMin.x = vertex->x;
        }
        if (vertex->y < outBoxMin.y)
        {
            outBoxMin.y = vertex->y;
        }
        if (vertex->z < outBoxMin.z)
        {
            outBoxMin.z = vertex->z;
        }

        if (vertex->x > outBoxMax.x)
        {
            outBoxMax.x = vertex->x;
        }
        if (vertex->y > outBoxMax.y)
        {
            outBoxMax.y = vertex->y;
        }
        if (vertex->z > outBoxMax.z)
        {
            outBoxMax.z = vertex->z;
        }
    }
}
