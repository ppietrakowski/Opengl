#include "static_mesh.h"
#include "Renderer.h"
#include "error_macros.h"
#include "assimp_utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

static void FindAabCollision(std::span<const StaticMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax) {
    // assume mesh has infinite bounds
    outBoxMin = glm::vec3{std::numeric_limits<float>::max()};
    outBoxMax = glm::vec3{std::numeric_limits<float>::min()};

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const glm::vec3* vertex = &vertices[i].position;

        if (vertex->x < outBoxMin.x) {
            outBoxMin.x = vertex->x;
        }
        if (vertex->y < outBoxMin.y) {
            outBoxMin.y = vertex->y;
        }
        if (vertex->z < outBoxMin.z) {
            outBoxMin.z = vertex->z;
        }

        if (vertex->x > outBoxMax.x) {
            outBoxMax.x = vertex->x;
        }
        if (vertex->y > outBoxMax.y) {
            outBoxMax.y = vertex->y;
        }
        if (vertex->z > outBoxMax.z) {
            outBoxMax.z = vertex->z;
        }
    }
}


StaticMesh::StaticMesh(const std::filesystem::path& file_path, const std::shared_ptr<Material>& material) :
    main_material{material},
    vertex_array_{VertexArray::Create()} {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path.string(), kAssimpImportFlags);

    if (scene == nullptr) {
        throw std::runtime_error{importer.GetErrorString()};
    }

    std::int32_t total_vertices = 0;
    std::int32_t total_indices = 0;

    vertices.reserve(scene->mMeshes[0]->mNumVertices);

    std::int32_t start_num_indices = scene->mMeshes[0]->mNumFaces * 3;
    indices.reserve(start_num_indices);

    for (std::uint32_t i = 0; i < scene->mNumMaterials; ++i) {
        aiString texturePath;

        if (scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0,
            &texturePath, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS) {
            texture_paths.emplace_back(texturePath.C_Str());
        }
    }

    for (std::uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];

        for (std::uint32_t j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D texture_coords = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(texture_coords), 0);
        }

        for (std::uint32_t j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3);

            for (std::uint32_t k = 0; k < face.mNumIndices; ++k) {
                indices.emplace_back(face.mIndices[k] + total_indices);
            }
        }

        total_vertices += mesh->mNumVertices;
        total_indices += mesh->mNumFaces * 3;
    }

    std::shared_ptr<IndexBuffer> index_buffer = IndexBuffer::Create(indices.data(),
        static_cast<std::int32_t>(indices.size()));

    vertex_array_->AddBuffer<StaticMeshVertex>(vertices, StaticMeshVertex::kDataFormat);
    vertex_array_->SetIndexBuffer(index_buffer);

    num_triangles_ = static_cast<std::int32_t>(indices.size()) / 3;
    mesh_name_ = scene->mName.C_Str();
    FindAabCollision(vertices, bbox_min_, bbox_max_);
}


void StaticMesh::Render(const glm::mat4& transform) const {
    Renderer::Submit(*main_material, *vertex_array_, transform);
}

void StaticMesh::Render(const Material& override_material, const glm::mat4& transform) const {
    Renderer::Submit(override_material, *vertex_array_, transform);
}

