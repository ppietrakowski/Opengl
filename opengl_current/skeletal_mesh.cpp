#include "skeletal_mesh.h"
#include "renderer.h"
#include "assimp_utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Core.h"

static void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& out_box_min, glm::vec3& out_box_max);

glm::mat4 Animation::GetBoneTransformOrRelative(const Bone& bone, float animation_time) const {
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
    position{ position },
    normal{ normal },
    texture_coords{ texture_coords } {}

bool SkeletonMeshVertex::AddBoneData(uint32_t bone_id, float weight) {
    // find first empty slot
    auto it = std::find(std::begin(bone_weights), std::end(bone_weights), 0.0f);

    if (it != std::end(bone_weights)) {
        std::ptrdiff_t index = std::distance(std::begin(bone_weights), it);
        bone_ids[index] = (float)bone_id;
        bone_weights[index] = weight;

        return true;
    }

    return false;
}

static constexpr const char* kDefaultAnimationName = "TPose";

SkeletalMesh::SkeletalMesh(const std::filesystem::path& path, const std::shared_ptr<Material>& material) :
    material_{ material },
    current_animation_name_{ kDefaultAnimationName },
    num_bones_{ 0 },
    vertex_array_{ VertexArray::Create() } {
    // maps bone name to boneID
    std::unordered_map<std::string, uint32_t> bone_name_to_index;

    auto get_bone_id = [&bone_name_to_index](const aiBone* bone) {
        uint32_t bone_id = 0;
        std::string bone_name(bone->mName.C_Str());

        auto it = bone_name_to_index.find(bone_name);

        bool bone_id_already_defined = it != bone_name_to_index.end();

        if (bone_id_already_defined) {
            bone_id = it->second;
        } else {
            bone_id = static_cast<uint32_t>(bone_name_to_index.size());
            bone_name_to_index[bone_name] = bone_id;
        }

        return bone_id;
    };

    uint32_t total_vertices = 0;
    uint32_t total_indices = 0;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path.string(), kAssimpImportFlags);
    CRASH_EXPECTED_NOT_NULL(scene);

    // used for reserve enough indices to decrease allocating overhead
    uint32_t start_num_indices = scene->mMeshes[0]->mNumFaces * 3u;

    // packed all vertices of all meshes in aiScene
    std::vector<SkeletonMeshVertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(scene->mMeshes[0]->mNumVertices);
    indices.reserve(start_num_indices);

    std::vector<std::shared_ptr<Texture2D>> textures;

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
        auto texture = LoadTexturesFromMaterial(scene, i);

        if (texture != nullptr) {
            textures.emplace_back(texture);
        }
    }

    std::unordered_map<std::string, BoneInfo> bones_info;

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D texture_coord = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(texture_coord));
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
            uint32_t bone_id = get_bone_id(bone);

            glm::mat4 offset_matrix = ToGlm(bone->mOffsetMatrix);
            bones_info[bone->mName.C_Str()] = BoneInfo{ bone_index, offset_matrix };
            std::string s{ bone->mName.C_Str() };

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

    material_->should_cull_faces = false;
    material_->using_transparency = true;

    // set diffuse textures
    for (uint32_t i = 0; i < textures.size(); ++i) {
        std::string name = "diffuse" + std::to_string(i + 1);
        material_->SetTextureProperty(name.c_str(), textures[i]);
    }

    for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
        LoadAnimation(scene, i);
    }

    // define Tpose animation dummy values
    animations_[current_animation_name_] = Animation{};
    animations_[current_animation_name_].ticks_per_second = 30;
    animations_[current_animation_name_].duration = 10;

    root_joint_.AssignHierarchy(scene->mRootNode, bones_info);
    vertex_array_->AddBuffer<SkeletonMeshVertex>(vertices, SkeletonMeshVertex::data_format);
    vertex_array_->SetIndexBuffer(IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size())));

    // find global transform for converting from bone space back to local space
    global_inverse_transform_ = glm::inverse(ToGlm(scene->mRootNode->mTransformation));
    bone_transforms_.resize(num_bones_, glm::identity<glm::mat4>());

    FindAabCollision(vertices, bbox_min_, bbox_max_);
    bbox_min_.x *= 0.2f;
    bbox_max_.x *= 0.2f;
}

void SkeletalMesh::LoadAnimation(const aiScene* scene, uint32_t animation_index) {
    const aiAnimation* anim = scene->mAnimations[animation_index];
    Animation animation{};

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

void SkeletalMesh::Draw(const glm::mat4& transform) {
    if (should_draw_debug_bounds) {
        Renderer::DrawDebugBox(bbox_min_, bbox_max_, transform);
    }

    Renderer::SubmitSkeleton(*material_, bone_transforms_, num_bones_, *vertex_array_, transform);
}

void SkeletalMesh::SetCurrentAnimation(const std::string& animation_name) {
    auto it = animations_.find(animation_name);

    if (it != animations_.end()) {
        current_animation_name_ = animation_name;
    }
}

std::vector<std::string> SkeletalMesh::GetAnimationNames() const {
    std::vector<std::string> names;

    names.reserve(animations_.size());

    for (auto& [name, animation] : animations_) {
        names.emplace_back(name);
    }

    return names;
}

void SkeletalMesh::CalculateTransform(const BoneAnimationUpdateSpecs& update_specs, const glm::mat4& parent_transform) {
    const Bone& joint = *update_specs.joint;
    glm::mat4 transform = update_specs->GetBoneTransformOrRelative(joint, update_specs.animation_time);

    uint32_t index = joint.bone_transform_index;
    glm::mat4 global_transform = parent_transform * transform;
    bone_transforms_[index] = global_inverse_transform_ * global_transform * joint.bone_offset;

    // run chain to update other joint transforms
    for (const Bone& child : joint.children) {
        BoneAnimationUpdateSpecs new_update_specs = update_specs;
        new_update_specs.joint = &child;
        CalculateTransform(new_update_specs, global_transform);
    }
}

std::shared_ptr<Texture2D> SkeletalMesh::LoadTexturesFromMaterial(const aiScene* scene, uint32_t material_index) {
    aiString texture_path;

    if (scene->mMaterials[material_index]->GetTexture(aiTextureType_DIFFUSE, 0,
        &texture_path, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS) {
        const aiTexture* texture = scene->GetEmbeddedTexture(texture_path.C_Str());

        if (texture != nullptr) {
            bool is_compressed = texture->mHeight == 0;

            if (is_compressed) {
                return Texture2D::CreateFromImage(LoadRgbaImageFromMemory(texture->pcData, texture->mWidth));
            } else {
                return Texture2D::CreateFromImage(LoadRgbaImageFromMemory(texture->pcData,
                    texture->mWidth * texture->mHeight));
            }
        }
    }

    return nullptr;
}

void FindAabCollision(std::span<const SkeletonMeshVertex> vertices, glm::vec3& out_box_min, glm::vec3& out_box_max) {
    // assume mesh has infinite bounds
    out_box_min = glm::vec3{ std::numeric_limits<float>::max() };
    out_box_max = glm::vec3{ std::numeric_limits<float>::min() };

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
