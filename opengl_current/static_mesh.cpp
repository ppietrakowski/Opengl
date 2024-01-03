#include "static_mesh.h"
#include "Renderer.h"
#include "error_macros.h"
#include "assimp_utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

static void FindAabCollision(std::span<const StaticMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax)
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


StaticMesh::StaticMesh(const std::filesystem::path& file_path, const std::shared_ptr<Material>& material) :
    MainMaterial{material},
    VertexArray{IVertexArray::Create()}
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path.string(), kAssimpImportFlags);

    if (scene == nullptr)
    {
        throw std::runtime_error{importer.GetErrorString()};
    }

    std::vector<StaticMeshVertex> vertices;
    std::vector<uint32_t> indices;
    int32_t totalVertices = 0;
    int32_t totalIndices = 0;

    vertices.reserve(scene->mMeshes[0]->mNumVertices);

    int32_t startNumIndices = scene->mMeshes[0]->mNumFaces * 3;
    indices.reserve(startNumIndices);

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        aiString texturePath;

        if (scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0,
            &texturePath, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS)
        {
            TexturePaths.emplace_back(texturePath.C_Str());
        }
    }

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j)
        {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D textureCoords = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(textureCoords));
        }

        for (uint32_t j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3);

            for (uint32_t k = 0; k < face.mNumIndices; ++k)
            {
                indices.emplace_back(face.mIndices[k] + totalIndices);
            }
        }

        totalVertices += mesh->mNumVertices;
        totalIndices += mesh->mNumFaces * 3;
    }

    std::shared_ptr<IIndexBuffer> indexBuffer = IIndexBuffer::Create(indices.data(),
        static_cast<int32_t>(indices.size()));

    VertexArray->AddBuffer<StaticMeshVertex>(vertices, StaticMeshVertex::kDataFormat);
    VertexArray->SetIndexBuffer(indexBuffer);

    NumTriangles = static_cast<int32_t>(indices.size()) / 3;
    MeshName = scene->mName.C_Str();
    FindAabCollision(vertices, BboxMin, BboxMax);
}


void StaticMesh::Render(const glm::mat4& transform) const
{
    Renderer::Submit(*MainMaterial, *VertexArray, transform);
}

void StaticMesh::Render(const Material& overrideMaterial, const glm::mat4& transform) const
{
    Renderer::Submit(overrideMaterial, *VertexArray, transform);
}

