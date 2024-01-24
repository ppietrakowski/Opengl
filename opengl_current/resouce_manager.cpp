#include "resouce_manager.h"
#include <fstream>
#include <filesystem>

std::shared_ptr<ResourceManagerImpl> ResourceManager::s_ResourceManagerInstance;

class ResourceManagerImpl
{
public:
    std::shared_ptr<Shader> GetShader(const std::string& filePath);
    std::shared_ptr<Shader> LoadShader(const std::string& filePath);

    std::shared_ptr<Texture2D> GetTexture2D(const std::string& filePath);
    std::shared_ptr<Texture2D> LoadTexture2D(const std::string& filePath);
    void AddTexture2D(const std::string& path, const std::shared_ptr<Texture2D>& texture);

    std::shared_ptr<SkeletalMesh> GetSkeletalMesh(const std::string& filePath);
    std::shared_ptr<SkeletalMesh> LoadSkeletalMesh(const std::string& filePath);

    std::shared_ptr<StaticMesh> GetStaticMesh(const std::string& filePath);
    std::shared_ptr<StaticMesh> LoadStaticMesh(const std::string& filePath);

    std::shared_ptr<Material> GetMaterial(const std::string& materialName);
    std::shared_ptr<Material> CreateMaterial(const std::string& shaderFilePath, const std::string& materialName);

private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures2d;
    std::unordered_map<std::string, std::shared_ptr<SkeletalMesh>> m_SkeletalMeshes;
    std::unordered_map<std::string, std::shared_ptr<StaticMesh>> m_StaticMeshes;
};

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& filePath)
{
    ASSERT(s_ResourceManagerInstance);
    return s_ResourceManagerInstance->GetShader(filePath);
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture2D(const std::string& filePath)
{
    ASSERT(s_ResourceManagerInstance);
    return s_ResourceManagerInstance->GetTexture2D(filePath);
}

void ResourceManager::AddTexture2D(const std::string& path, const std::shared_ptr<Texture2D>& texture)
{
    ASSERT(s_ResourceManagerInstance);
    return s_ResourceManagerInstance->AddTexture2D(path, texture);
}

std::shared_ptr<SkeletalMesh> ResourceManager::GetSkeletalMesh(const std::string& filePath)
{
    ASSERT(s_ResourceManagerInstance);
    return s_ResourceManagerInstance->GetSkeletalMesh(filePath);
}

std::shared_ptr<StaticMesh> ResourceManager::GetStaticMesh(const std::string& filePath)
{
    ASSERT(s_ResourceManagerInstance);
    return s_ResourceManagerInstance->GetStaticMesh(filePath);
}

std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& materialName)
{
    ASSERT(s_ResourceManagerInstance);
    return s_ResourceManagerInstance->GetMaterial(materialName);
}

std::shared_ptr<Material> ResourceManager::CreateMaterial(const std::string& shaderFilePath, const std::string& materialName)
{
    ASSERT(s_ResourceManagerInstance);
    return s_ResourceManagerInstance->CreateMaterial(shaderFilePath, materialName);
}

void ResourceManager::Quit()
{
    s_ResourceManagerInstance = nullptr;
}

std::shared_ptr<ResourceManagerImpl> ResourceManager::CreateResourceManager()
{
    s_ResourceManagerInstance = std::make_shared<ResourceManagerImpl>();
    return s_ResourceManagerInstance;
}

std::shared_ptr<Shader> ResourceManagerImpl::GetShader(const std::string& filePath)
{
    auto it = m_Shaders.find(filePath);

    if (it == m_Shaders.end())
    {
        return LoadShader(filePath);
    }

    return it->second;
}

struct ShaderStringMatch
{
    char Tag[32];
    ShaderIndex::IndexType Index;
};

std::shared_ptr<Shader> ResourceManagerImpl::LoadShader(const std::string& filePath)
{
    std::string line;
    std::fstream file(filePath);
    std::vector<std::string> shaderSources;

    ASSERT(file.good() || !file.fail());

    shaderSources.reserve(ShaderIndex::Count);
    std::filesystem::path path = filePath;
    path = path.remove_filename();

    auto insertAt = [&shaderSources](const std::string& source, std::int32_t index)
    {
        if (shaderSources.size() <= index)
        {
            std::int32_t newSize = index + 1;
            shaderSources.resize(newSize);
        }

        shaderSources[index] = source;
    };

    const ShaderStringMatch StringMatches[] =
    {
        {"VertexShader", ShaderIndex::Vertex},
        {"FragmentShader", ShaderIndex::Fragment},
        {"GeometryShader", ShaderIndex::Geometry},
        {"TesselationControlShader", ShaderIndex::TesselationControlShader},
        {"TesselationEvaluationShader", ShaderIndex::TesselationEvaluateShader},
    };

    while (std::getline(file, line))
    {
        std::vector<std::string> manifestLine = SplitString(line, "=");

        if (manifestLine.empty())
        {
            continue;
        }

        manifestLine[1] = path.string() + manifestLine[1];

        for (const ShaderStringMatch& match : StringMatches)
        {
            if (manifestLine[0] == match.Tag)
            {
                insertAt(LoadFileContent(manifestLine[1]), match.Index);
                break;
            }
        }
    }

    ASSERT(shaderSources.size() < ShaderIndex::Count);

    std::shared_ptr<Shader> shader = std::make_shared<Shader>(shaderSources);
    m_Shaders[filePath] = shader;
    return shader;
}

std::shared_ptr<Texture2D> ResourceManagerImpl::GetTexture2D(const std::string& filePath)
{
    auto it = m_Textures2d.find(filePath);

    if (it == m_Textures2d.end())
    {
        return LoadTexture2D(filePath);
    }

    return it->second;
}

std::shared_ptr<Texture2D> ResourceManagerImpl::LoadTexture2D(const std::string& filePath)
{
    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(filePath);
    m_Textures2d[filePath] = texture;

    return texture;
}

void ResourceManagerImpl::AddTexture2D(const std::string& path, const std::shared_ptr<Texture2D>& texture)
{
    m_Textures2d[path] = texture;
}

std::shared_ptr<SkeletalMesh> ResourceManagerImpl::GetSkeletalMesh(const std::string& filePath)
{
    auto it = m_SkeletalMeshes.find(filePath);

    if (it == m_SkeletalMeshes.end())
    {
        return LoadSkeletalMesh(filePath);
    }

    return it->second;
}

std::shared_ptr<SkeletalMesh> ResourceManagerImpl::LoadSkeletalMesh(const std::string& filePath)
{
    std::shared_ptr<SkeletalMesh> skeletalMesh = std::make_shared<SkeletalMesh>(filePath, GetMaterial("default"));
    m_SkeletalMeshes[filePath] = skeletalMesh;
    return skeletalMesh;
}

std::shared_ptr<StaticMesh> ResourceManagerImpl::GetStaticMesh(const std::string& filePath)
{
    auto it = m_StaticMeshes.find(filePath);

    if (it == m_StaticMeshes.end())
    {
        return LoadStaticMesh(filePath);
    }

    return it->second;
}

std::shared_ptr<StaticMesh> ResourceManagerImpl::LoadStaticMesh(const std::string& filePath)
{
    std::shared_ptr<StaticMesh> staticMesh = std::make_shared<StaticMesh>(filePath, GetMaterial("default"));
    m_StaticMeshes[filePath] = staticMesh;
    return staticMesh;
}

std::shared_ptr<Material> ResourceManagerImpl::GetMaterial(const std::string& materialName)
{
    auto it = m_Materials.find(materialName);

    if (it == m_Materials.end())
    {
        THROW_ERROR("Couldn't find material with such name");
    }

    return it->second;
}

std::shared_ptr<Material> ResourceManagerImpl::CreateMaterial(const std::string& shaderFilePath, const std::string& materialName)
{
    auto shader = GetShader(shaderFilePath);
    m_Materials[materialName] = std::make_shared<Material>(shader);
    return m_Materials[materialName];
}

