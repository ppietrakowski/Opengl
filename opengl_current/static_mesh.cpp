#include "static_mesh.h"
#include "Renderer.h"
#include "error_macros.h"
#include "assimp_utils.h"

#include "resouce_manager.h"

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

StaticMesh::StaticMesh(const std::filesystem::path& filePath, const std::shared_ptr<Material>& material) :
    MainMaterial{material}
{
    LoadLod(filePath.string(), 0);
}

void StaticMesh::LoadLod(const std::string& filePath, int lod)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, AssimpImportFlags);

    if (scene == nullptr)
    {
        throw std::runtime_error{importer.GetErrorString()};
    }

    int totalVertices = 0;
    int totalIndices = 0;

    std::vector<StaticMeshVertex> vertices;
    vertices.reserve(scene->mMeshes[0]->mNumVertices);

    int startNumIndices = scene->mMeshes[0]->mNumFaces * 3;
    std::vector<uint32_t> indices;
    indices.reserve(startNumIndices);

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        aiString texturePaths;

        if (scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0,
            &texturePaths, nullptr, nullptr, nullptr, nullptr, nullptr) == aiReturn_SUCCESS)
        {
            const aiTexture* texture = scene->GetEmbeddedTexture(texturePaths.C_Str());
            bool is_compressed = texture->mHeight == 0;

            int length = static_cast<int>(texture->mWidth);

            if (!is_compressed)
            {
                length *= texture->mHeight;
            }

            ResourceManager::AddTexture2D(texturePaths.C_Str(), std::make_shared<Texture2D>(
                LoadRgbaImageFromMemory(texture->pcData, length)));

            TextureNames.push_back(texturePaths.C_Str());
        }
    }

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];

        for (uint32_t j = 0; j < mesh->mNumVertices; ++j)
        {
            const aiVector3D& pos = mesh->mVertices[j];
            const aiVector3D& normal = mesh->mNormals[j];
            const aiVector3D& textureCoords = mesh->mTextureCoords[0][j];

            vertices.emplace_back(ToGlm(pos), ToGlm(normal), ToGlm(textureCoords), 0);
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

    if (m_Entries.empty())
    {
        m_MeshName = scene->mName.C_Str();
        FindAabCollision(vertices, m_BoundingBox.MinBounds, m_BoundingBox.MaxBounds);
    }

    if (m_Entries.size() == lod)
    {
        m_Entries.emplace_back(vertices, indices);
    }
    else if (m_Entries.size() >= lod)
    {
        m_Entries[lod] = StaticMeshEntry(vertices, indices);
    }
}

StaticMeshEntry::StaticMeshEntry(const std::vector<StaticMeshVertex>& vertices, const std::vector<uint32_t>& indices) :
    m_VertexArray(std::make_shared<VertexArray>()),
    Vertices(vertices),
    Indices(indices)
{
    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(Indices.data(),
        STD_ARRAY_NUM_ELEMENTS(Indices));

    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(Vertices.data(),
        STD_ARRAY_NUM_BYTES(Vertices, StaticMeshVertex));

    m_VertexArray->AddVertexBuffer(vertexBuffer, StaticMeshVertex::DataFormat);
    m_VertexArray->SetIndexBuffer(indexBuffer);

    m_NumTriangles = STD_ARRAY_NUM_ELEMENTS(Indices) / 3;
}
