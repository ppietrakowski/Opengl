#include "Mesh.h"
#include "Renderer.h"
#include "ErrorMacros.h"
#include "AssimpUtils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

static void FindAabCollision(std::span<const StaticMeshVertex> vertices, glm::vec3& outBoxMin, glm::vec3& outBoxMax)
{
    // assume mesh has infinite bounds
    outBoxMin = glm::vec3{ std::numeric_limits<float>::max() };
    outBoxMax = glm::vec3{ std::numeric_limits<float>::min() };

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


StaticMesh::StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material) :
    _material{ material }
{

    Assimp::Importer importer;
    constexpr std::uint32_t ImportFlags = aiProcess_Triangulate |
        aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs;

    const aiScene* scene = importer.ReadFile(filePath.string(), ImportFlags);

    if (scene == nullptr)
    {
        throw std::runtime_error{ importer.GetErrorString() };
    }

    std::uint32_t totalVertices = 0;
    std::uint32_t totalIndices = 0;

    _vertices.reserve(scene->mMeshes[0]->mNumVertices);
    _indices.reserve(scene->mMeshes[0]->mNumFaces * 3);

    for (std::uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
    {
        aiString path;

        if (scene->mMaterials[materialIndex]->GetTexture(aiTextureType_DIFFUSE, 0,
            &path, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS)
        {
            TexturePaths.push_back(path.C_Str());
        }
    }

    for (std::uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        
        for (std::uint32_t j = 0; j < mesh->mNumVertices; ++j)
        {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D normal = mesh->mNormals[j];
            aiVector3D textureCoord = mesh->mTextureCoords[0][j];

            _vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(textureCoord));
        }

        for (std::uint32_t j = 0; j < mesh->mNumFaces; ++j)
        {
            const aiFace& face = mesh->mFaces[j];
            ASSERT(face.mNumIndices == 3);

            for (std::uint32_t k = 0; k < 3; ++k)
            {
                _indices.emplace_back(face.mIndices[k] + totalIndices);
            }
        }

        totalVertices += mesh->mNumVertices;
        totalIndices += mesh->mNumFaces * 3;
    }

    IndexBuffer indexBuffer(_indices.data(),
        static_cast<std::uint32_t>(_indices.size()));

    _vertexArray.AddBuffer<StaticMeshVertex>(_vertices, StaticMeshVertex::DataFormat);
    _vertexArray.SetIndexBuffer(std::move(indexBuffer));

    _numTriangles = static_cast<std::uint32_t>(_indices.size()) / 3;
    _meshName = scene->mName.C_Str();
    FindAabCollision(_vertices, _bboxMin, _bboxMax);
}


void StaticMesh::Render(const glm::mat4& transform) const
{
    Renderer::Submit(*_material, _vertexArray, transform);
}

void StaticMesh::Render(const Material& overrideMaterial, const glm::mat4& transform) const
{
    Renderer::Submit(overrideMaterial, _vertexArray, transform);
}

