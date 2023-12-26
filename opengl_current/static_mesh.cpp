#include "static_mesh.h"
#include "Renderer.h"
#include "error_macros.h"
#include "assimp_utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

static void FindAabCollision(std::span<const StaticMeshVertex> vertices, glm::vec3& out_box_min, glm::vec3& out_box_max) {
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


StaticMesh::StaticMesh(const std::filesystem::path& file_path, const std::shared_ptr<Material>& material) :
    material_{ material },
    vertex_array_{ VertexArray::Create() } {

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path.string(), kAssimpImportFlags);

    if (scene == nullptr) {
        throw std::runtime_error{ importer.GetErrorString() };
    }

    std::vector<StaticMeshVertex> vertices_;
    std::vector<uint32_t> indices;
    uint32_t total_vertices = 0;
    uint32_t total_indices = 0;

    vertices_.reserve(scene->mMeshes[0]->mNumVertices);
    
    uint32_t start_num_indices = scene->mMeshes[0]->mNumFaces * 3;
    indices.reserve(start_num_indices);

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
        aiString texture_path;

        if (scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0,
            &texture_path, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS) {
            texture_paths.emplace_back(texture_path.C_Str());
        }
    }

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D texture_coords = mesh->mTextureCoords[0][j];

            vertices_.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(texture_coords));
        }

        for (uint32_t j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3);

            for (uint32_t k = 0; k < 3; ++k) {
                indices.emplace_back(face.mIndices[k] + total_indices);
            }
        }

        total_vertices += mesh->mNumVertices;
        total_indices += mesh->mNumFaces * 3;
    }

    std::shared_ptr<IndexBuffer> index_buffer = IndexBuffer::Create(indices.data(),
        static_cast<uint32_t>(indices.size()));
    
    vertex_array_->AddBuffer<StaticMeshVertex>(vertices_, StaticMeshVertex::data_format);
    vertex_array_->SetIndexBuffer(index_buffer);

    num_triangles_ = static_cast<uint32_t>(indices.size()) / 3;
    mesh_name_ = scene->mName.C_Str();
    FindAabCollision(vertices_, bbox_min_, bbox_max_);
}


void StaticMesh::Render(const glm::mat4& transform) const {
    Renderer::Submit(*material_, *vertex_array_, transform);
}

void StaticMesh::Render(const Material& override_material, const glm::mat4& transform) const {
    Renderer::Submit(override_material, *vertex_array_, transform);
}

