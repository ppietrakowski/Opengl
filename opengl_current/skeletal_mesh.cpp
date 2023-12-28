#include "skeletal_mesh.h"
#include "renderer.h"
#include "assimp_utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Core.h"

static void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax);

glm::mat4 Animation::GetBoneTransformOrRelative(const Bone& bone, float animationTime) const
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
    bool bIsBone = it != bonesInfo.end();

    if (bIsBone)
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

bool SkeletonMeshVertex::AddBoneData(int32_t boneId, float weight)
{
    // find first empty slot
    auto it = std::find(std::begin(BoneWeights), std::end(BoneWeights), 0.0f);

    if (it != std::end(BoneWeights))
    {
        std::ptrdiff_t index = std::distance(std::begin(BoneWeights), it);
        BoneIds[index] = boneId;
        BoneWeights[index] = weight;

        return true;
    }

    return false;
}

static constexpr const char* kDefaultAnimationName = "TPose";

SkeletalMesh::SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material) :
    m_Material{material},
    m_CurrentAnimationName{kDefaultAnimationName},
    m_NumBones{0},
    m_VertexArray{IVertexArray::Create()}
{
    // maps bone name to boneID
    std::unordered_map<std::string, int32_t> boneNameToIndex;

    auto getBoneId = [&boneNameToIndex](const aiBone* bone) {
        int32_t boneId = 0;
        std::string boneName(bone->mName.C_Str());

        auto it = boneNameToIndex.find(boneName);

        bool bBoneIdAlreadyDefined = it != boneNameToIndex.end();

        if (bBoneIdAlreadyDefined)
        {
            boneId = it->second;
        }
        else
        {
            boneId = static_cast<int32_t>(boneNameToIndex.size());
            boneNameToIndex[boneName] = boneId;
        }

        return boneId;
    };

    int32_t totalVertices = 0;
    int32_t totalIndices = 0;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.string(), kAssimpImportFlags);
    CRASH_EXPECTED_NOT_NULL(scene);

    // used for reserve enough indices to decrease allocating overhead
    int32_t startNumIndices = scene->mMeshes[0]->mNumFaces * 3;

    // packed all vertices of all meshes in aiScene
    std::vector<SkeletonMeshVertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(scene->mMeshes[0]->mNumVertices);
    indices.reserve(startNumIndices);

    std::vector<std::shared_ptr<ITexture2D>> textures;

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        auto texture = LoadTexturesFromMaterial(scene, i);

        if (texture != nullptr)
        {
            textures.emplace_back(texture);
        }
    }

    std::unordered_map<std::string, BoneInfo> bonesInfo;

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j)
        {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D texture_coord = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(texture_coord));
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

        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            const aiBone* bone = mesh->mBones[boneIndex];
            int32_t boneId = getBoneId(bone);

            glm::mat4 offsetMatrix = ToGlm(bone->mOffsetMatrix);
            bonesInfo[bone->mName.C_Str()] = BoneInfo{static_cast<int32_t>(boneIndex), offsetMatrix};
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

    m_Material->bCullFaces = false;
    m_Material->bTransparent = true;

    // set diffuse textures
    for (uint32_t i = 0; i < textures.size(); ++i)
    {
        std::string name = "diffuse" + std::to_string(i + 1);
        m_Material->SetTextureProperty(name.c_str(), textures[i]);
    }

    for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        LoadAnimation(scene, i);
    }

    // define Tpose animation dummy values
    m_Animations[m_CurrentAnimationName] = Animation{};
    m_Animations[m_CurrentAnimationName].TicksPerSecond = 30;
    m_Animations[m_CurrentAnimationName].Duration = 10;

    m_RootBone.AssignHierarchy(scene->mRootNode, bonesInfo);
    m_VertexArray->AddBuffer<SkeletonMeshVertex>(vertices, SkeletonMeshVertex::kDataFormat);
    m_VertexArray->SetIndexBuffer(IIndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size())));

    // find global transform for converting from bone space back to local space
    m_GlobalInverseTransform = glm::inverse(ToGlm(scene->mRootNode->mTransformation));
    m_BoneTransforms.resize(m_NumBones, glm::identity<glm::mat4>());

    FindAabCollision(vertices, m_BboxMin, m_BboxMax);
    m_BboxMin.x *= 0.2f;
    m_BboxMax.x *= 0.2f;
}

void SkeletalMesh::LoadAnimation(const aiScene* scene, int32_t animationIndex)
{
    const aiAnimation* anim = scene->mAnimations[animationIndex];
    Animation animation{};

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

void SkeletalMesh::Draw(const glm::mat4& transform)
{
    if (bShouldDrawDebugBounds)
    {
        Renderer::DrawDebugBox(m_BboxMin, m_BboxMax, transform);
    }

    Renderer::SubmitSkeleton(*m_Material, m_BoneTransforms, m_NumBones, *m_VertexArray, transform);
}

void SkeletalMesh::SetCurrentAnimation(const std::string& animationName)
{
    auto it = m_Animations.find(animationName);

    if (it != m_Animations.end())
    {
        m_CurrentAnimationName = animationName;
    }
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

void SkeletalMesh::CalculateTransform(const BoneAnimationUpdateSpecs& updateSpecs, const glm::mat4& parentTransform)
{
    const Bone& joint = *updateSpecs.Joint;
    glm::mat4 transform = updateSpecs->GetBoneTransformOrRelative(joint, updateSpecs.AnimationTime);

    int32_t index = joint.BoneTransformIndex;
    glm::mat4 globalTransform = parentTransform * transform;
    m_BoneTransforms[index] = m_GlobalInverseTransform * globalTransform * joint.BoneOffset;

    // run chain to update other joint transforms
    for (const Bone& child : joint.Children)
    {
        BoneAnimationUpdateSpecs newUpdateSpecs = updateSpecs;
        newUpdateSpecs.Joint = &child;
        CalculateTransform(newUpdateSpecs, globalTransform);
    }
}

std::shared_ptr<ITexture2D> SkeletalMesh::LoadTexturesFromMaterial(const aiScene* scene, int32_t materialIndex)
{
    aiString texturePath;

    if (scene->mMaterials[materialIndex]->GetTexture(aiTextureType_DIFFUSE, 0,
        &texturePath, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS)
    {
        const aiTexture* texture = scene->GetEmbeddedTexture(texturePath.C_Str());

        if (texture != nullptr)
        {
            bool bIsCompressed = texture->mHeight == 0;

            if (bIsCompressed)
            {
                return ITexture2D::CreateFromImage(LoadRgbaImageFromMemory(texture->pcData, texture->mWidth));
            }
            else
            {
                return ITexture2D::CreateFromImage(LoadRgbaImageFromMemory(texture->pcData,
                    texture->mWidth * texture->mHeight));
            }
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
