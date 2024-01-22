#include "skeletal_mesh.h"
#include "renderer.h"
#include "assimp_utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Core.h"
#include "resouce_manager.h"

static void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& out_box_min, glm::vec3& out_box_max);

glm::mat4 SkeletalAnimation::GetBoneTransformOrRelative(const Bone& bone, float animation_time) const {
    auto it = bone_name_to_tracks.find(bone.name);

    if (it == bone_name_to_tracks.end()) {
        // track for this bone could not be found, so take default relative_transform_matrix
        return bone.relative_transform_matrix;
    }

    const BoneAnimationTrack& track = it->second;
    glm::vec3 position = track.Interpolate<glm::vec3>(animation_time);
    glm::quat rotation = track.Interpolate<glm::quat>(animation_time);
    return glm::translate(position) * glm::mat4_cast(rotation);
}

bool Bone::AssignHierarchy(const aiNode* node, const std::unordered_map<std::string, BoneInfo>& bones_info) {
    auto it = bones_info.find(node->mName.C_Str());
    bool is_bone = it != bones_info.end();

    if (is_bone) {
        name = node->mName.C_Str();
        const BoneInfo& bone_info = it->second;

        bone_transform_index = bone_info.bone_transform_index;
        relative_transform_matrix = ToGlm(node->mTransformation);
        bone_offset = bone_info.offset_matrix;

        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            Bone child;
            child.AssignHierarchy(node->mChildren[i], bones_info);
            children.emplace_back(child);
        }

        return true;
    } else {
        // traverse all children to find bone
        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            if (AssignHierarchy(node->mChildren[i], bones_info)) {
                return true;
            }
        }
    }

    // traversed all children and it's not a bone
    return false;
}

SkeletonMeshVertex::SkeletonMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture_coords) :
    position{position},
    normal{normal},
    texture_coords{texture_coords} {
}

bool SkeletonMeshVertex::AddBoneData(int bone_id, float weight) {
    // find first empty slot
    auto it = std::find(std::begin(bone_weights), std::end(bone_weights), 0.0f);

    if (it != std::end(bone_weights)) {
        ptrdiff_t index = std::distance(std::begin(bone_weights), it);
        bone_ids[index] = bone_id;
        bone_weights[index] = weight;

        return true;
    }

    return false;
}

SkeletalMesh::SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material) :
    main_material{material},
    num_bones_{0},
    vertex_array_{std::make_shared<VertexArray>()} {
    // maps bone name to boneID
    std::unordered_map<std::string, int> bone_name_to_index;

    auto getBoneId = [&bone_name_to_index](const aiBone* bone) {
        int bone_id = 0;
        std::string bone_name(bone->mName.C_Str());

        auto it = bone_name_to_index.find(bone_name);

        bool bone_id_already_defined = it != bone_name_to_index.end();

        if (bone_id_already_defined) {
            bone_id = it->second;
        } else {
            bone_id = static_cast<int>(bone_name_to_index.size());
            bone_name_to_index[bone_name] = bone_id;
        }

        return bone_id;
    };

    int total_vertices = 0;
    int total_indices = 0;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.string(), kAssimpImportFlags);
    CRASH_EXPECTED_NOT_NULL(scene);

    // used for reserve enough indices to decrease allocating overhead
    int start_num_indices = scene->mMeshes[0]->mNumFaces * 3;

    // packed all vertices of all meshes in aiScene
    std::vector<SkeletonMeshVertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(scene->mMeshes[0]->mNumVertices);
    indices.reserve(start_num_indices);

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
        LoadTexturesFromMaterial(scene, i);
    }

    std::unordered_map<std::string, BoneInfo> bones_info;

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
            const aiVector3D& pos = mesh->mVertices[j];
            const aiVector3D& normal = mesh->mNormals[j];
            const aiVector3D& texture_coords = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(texture_coords));
            vertices.back().texture_id = mesh->mMaterialIndex;
        }

        for (uint32_t j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3 && "Face is not triangulated");

            for (uint32_t k = 0; k < face.mNumIndices; ++k) {
                indices.emplace_back(face.mIndices[k] + total_indices);
            }
        }

        for (uint32_t bone_index = 0; bone_index < mesh->mNumBones; ++bone_index) {
            const aiBone* bone = mesh->mBones[bone_index];
            int bone_id = getBoneId(bone);

            glm::mat4 offset_matrix = ToGlm(bone->mOffsetMatrix);
            bones_info[bone->mName.C_Str()] = BoneInfo{static_cast<int>(bone_index), offset_matrix};
            std::string s{bone->mName.C_Str()};

            for (uint32_t j = 0; j < bone->mNumWeights; j++) {
                // find global id of vertex
                uint32_t id = bone->mWeights[j].mVertexId + total_vertices;
                float weight = bone->mWeights[j].mWeight;

                if (!vertices[id].AddBoneData(bone_id, weight)) {
                    break;
                }
            }
        }

        total_vertices += mesh->mNumVertices;
        total_indices += mesh->mNumFaces * 3;
        num_bones_ += mesh->mNumBones;
    }

    main_material->transparent = true;

    for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
        LoadAnimation(scene, i);
    }

    // define Tpose animation dummy values
    animations_[kDefaultAnimationName] = SkeletalAnimation{};
    animations_[kDefaultAnimationName].ticks_per_second = 30;
    animations_[kDefaultAnimationName].duration = 10;

    root_bone_.AssignHierarchy(scene->mRootNode, bones_info);
    vertex_array_->AddVertexBuffer(std::make_shared<VertexBuffer>(vertices.data(), static_cast<int>(vertices.size() * sizeof(SkeletonMeshVertex))), SkeletonMeshVertex::kDataFormat);
    vertex_array_->SetIndexBuffer(std::make_shared<IndexBuffer>(indices.data(), static_cast<int>(indices.size())));

    // find global transform for converting from bone space back to local space
    global_inverse_transform_ = glm::inverse(ToGlm(scene->mRootNode->mTransformation));

    FindAabCollision(vertices, bounding_box_.min_bounds, bounding_box_.max_bounds);
    bounding_box_.min_bounds *= 0.01f;
    bounding_box_.max_bounds *= 0.01f;

    bounding_box_.min_bounds.x *= 0.2f;
    bounding_box_.max_bounds.x *= 0.2f;
}

void SkeletalMesh::LoadAnimation(const aiScene* scene, int anim_index) {
    const aiAnimation* anim = scene->mAnimations[anim_index];
    SkeletalAnimation animation{};

    if (anim->mTicksPerSecond != 0.0f) {
        animation.ticks_per_second = static_cast<float>(anim->mTicksPerSecond);
    } else {
        animation.ticks_per_second = 1.0f;
    }

    animation.duration = static_cast<float>(anim->mDuration);

    for (uint32_t i = 0; i < anim->mNumChannels; i++) {
        const aiNodeAnim* channel = anim->mChannels[i];
        BoneAnimationTrack track;

        for (uint32_t j = 0; j < channel->mNumPositionKeys; j++) {
            track.AddNewPositionTimestamp(ToGlm(channel->mPositionKeys[j].mValue),
                static_cast<float>(channel->mPositionKeys[j].mTime));
        }
        for (uint32_t j = 0; j < channel->mNumRotationKeys; j++) {
            track.AddNewRotationTimestamp(ToGlm(channel->mRotationKeys[j].mValue),
                static_cast<float>(channel->mRotationKeys[j].mTime));
        }

        // skip scale tracks, as it's not common to use scaling tracks of bones

        std::string name = channel->mNodeName.C_Str();
        animation.bone_name_to_tracks[name] = track;
    }

    std::string animation_name = anim->mName.C_Str();
    animation_name = SplitString(animation_name, "|").back();
    animations_[animation_name] = animation;
}

void SkeletalMesh::Draw(const std::vector<glm::mat4>& transforms, const glm::mat4& world_transform) {
    Renderer::SubmitSkeleton(SubmitCommandArgs{main_material.get(), 0, vertex_array_.get(), world_transform}, transforms);
}

std::vector<std::string> SkeletalMesh::GetAnimationNames() const {
    std::vector<std::string> names;

    names.reserve(animations_.size());

    for (auto& [name, animation] : animations_) {
        names.emplace_back(name);
    }

    return names;
}

void SkeletalMesh::GetAnimationFrames(const AnimationUpdateArgs& update_args) const {
    ASSERT(update_args.transforms.size() >= num_bones_);
    UpdateAnimation(update_args);
}

void SkeletalMesh::CalculateTransform(const BoneAnimationUpdateArgs& update_specs) const {
    const Bone& bone = *update_specs.target_bone;
    glm::mat4 transform = update_specs->GetBoneTransformOrRelative(bone, update_specs.animation_time);

    int index = bone.bone_transform_index;
    glm::mat4 global_transform = update_specs.parent_transform * transform;
    update_specs.UpdateTransformAt(index, global_inverse_transform_ * global_transform * bone.bone_offset);

    // run chain to update other joint transforms
    for (const Bone& child : bone.children) {
        BoneAnimationUpdateArgs new_update_specs = update_specs;
        new_update_specs.target_bone = &child;
        new_update_specs.parent_transform = global_transform;

        CalculateTransform(new_update_specs);
    }
}

std::shared_ptr<Texture2D> SkeletalMesh::LoadTexturesFromMaterial(const aiScene* scene, int material_index) {
    aiString texture_path;

    if (scene->mMaterials[material_index]->GetTexture(aiTextureType_DIFFUSE, 0,
        &texture_path, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS) {
        const aiTexture* texture = scene->GetEmbeddedTexture(texture_path.C_Str());

        if (texture != nullptr) {
            bool is_compressed = texture->mHeight == 0;

            if (is_compressed) {
                ResourceManager::AddTexture2D(texture_path.C_Str(),
                    std::make_shared<Texture2D>(LoadRgbaImageFromMemory(texture->pcData, texture->mWidth)));
            } else {
                ResourceManager::AddTexture2D(texture_path.C_Str(), std::make_shared<Texture2D>(
                    LoadRgbaImageFromMemory(texture->pcData, texture->mWidth * texture->mHeight)));
            }

            texture_names.emplace_back(texture_path.C_Str());

            return ResourceManager::GetTexture2D(texture_path.C_Str());
        }
    }

    return nullptr;
}

void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& out_box_min, glm::vec3& out_box_max) {
    // assume mesh has infinite bounds
    out_box_min = glm::vec3{std::numeric_limits<float>::max()};
    out_box_max = glm::vec3{std::numeric_limits<float>::min()};

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const glm::vec3* vertex = &vertices[i].position;

        if (vertex->x < out_box_min.x) {
            out_box_min.x = vertex->x;
        }
        if (vertex->y < out_box_min.y) {
            out_box_min.y = vertex->y;
        }
        if (vertex->z < out_box_min.z) {
            out_box_min.z = vertex->z;
        }

        if (vertex->x > out_box_max.x) {
            out_box_max.x = vertex->x;
        }
        if (vertex->y > out_box_max.y) {
            out_box_max.y = vertex->y;
        }
        if (vertex->z > out_box_max.z) {
            out_box_max.z = vertex->z;
        }
    }
}
