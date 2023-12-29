#include "resouce_manager.h"
#include <fstream>
#include <filesystem>

std::shared_ptr<ResourceManagerImpl> ResourceManager::s_ResourceManager;

class ResourceManagerImpl
{
public:
    std::shared_ptr<IShader> GetShader(const std::string& filePath);
    std::shared_ptr<IShader> LoadShader(const std::string& filePath);

    std::shared_ptr<ITexture2D> GetTexture2D(const std::string& filePath);
    std::shared_ptr<ITexture2D> LoadTexture2D(const std::string& filePath);
    void AddTexture2D(const std::string& path, const std::shared_ptr<ITexture2D>& texture);

    std::shared_ptr<SkeletalMesh> GetSkeletalMesh(const std::string& filePath);
    std::shared_ptr<SkeletalMesh> LoadSkeletalMesh(const std::string& filePath);

    std::shared_ptr<StaticMesh> GetStaticMesh(const std::string& filePath);
    std::shared_ptr<StaticMesh> LoadStaticMesh(const std::string& filePath);

    std::shared_ptr<Material> GetMaterial(const std::string& materialName);
    std::shared_ptr<Material> CreateMaterial(const std::string& shaderFilePath, const std::string& materialName);

private:
    std::unordered_map<std::string, std::shared_ptr<IShader>> m_Shaders;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
    std::unordered_map<std::string, std::shared_ptr<ITexture2D>> m_Textures2D;
    std::unordered_map<std::string, std::shared_ptr<SkeletalMesh>> m_SkeletalMeshes;
    std::unordered_map<std::string, std::shared_ptr<StaticMesh>> m_StaticMeshes;
};

std::shared_ptr<IShader> ResourceManager::GetShader(const std::string& filePath)
{
    ASSERT(s_ResourceManager);
    return s_ResourceManager->GetShader(filePath);
}

std::shared_ptr<ITexture2D> ResourceManager::GetTexture2D(const std::string& filePath)
{
    ASSERT(s_ResourceManager);
    return s_ResourceManager->GetTexture2D(filePath);
}

void ResourceManager::AddTexture2D(const std::string& path, const std::shared_ptr<ITexture2D>& texture)
{
    ASSERT(s_ResourceManager);
    return s_ResourceManager->AddTexture2D(path, texture);
}

std::shared_ptr<SkeletalMesh> ResourceManager::GetSkeletalMesh(const std::string& filePath)
{
    ASSERT(s_ResourceManager);
    return s_ResourceManager->GetSkeletalMesh(filePath);
}

std::shared_ptr<StaticMesh> ResourceManager::GetStaticMesh(const std::string& filePath)
{
    ASSERT(s_ResourceManager);
    return s_ResourceManager->GetStaticMesh(filePath);
}

std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& materialName)
{
    ASSERT(s_ResourceManager);
    return s_ResourceManager->GetMaterial(materialName);
}

std::shared_ptr<Material> ResourceManager::CreateMaterial(const std::string& shaderFilePath, const std::string& materialName)
{
    ASSERT(s_ResourceManager);
    return s_ResourceManager->CreateMaterial(shaderFilePath, materialName);
}

void ResourceManager::Quit()
{
    s_ResourceManager = nullptr;
}

std::shared_ptr<ResourceManagerImpl> ResourceManager::CreateResourceManager()
{
    s_ResourceManager = std::make_shared<ResourceManagerImpl>();
    return s_ResourceManager;
}

std::shared_ptr<IShader> ResourceManagerImpl::GetShader(const std::string& filePath)
{
    auto it = m_Shaders.find(filePath);

    if (it == m_Shaders.end())
    {
        return LoadShader(filePath);
    }

    return it->second;
}

std::shared_ptr<IShader> ResourceManagerImpl::LoadShader(const std::string& filePath)
{
    std::string line;
    std::fstream file(filePath);
    std::vector<std::string> shaderSources;

    ASSERT(file.good() || !file.fail());

    shaderSources.reserve(IShader::kCount);
    std::filesystem::path path = filePath;
    path = path.remove_filename();

    auto insertAt = [&shaderSources](const std::string& source, int32_t index) {
        if (shaderSources.size() <= index)
        {
            shaderSources.resize(index + 1);
        }

        shaderSources[index] = source;
    };

    while (std::getline(file, line))
    {
        std::vector<std::string> manifestFiles = SplitString(line, "=");

        if (manifestFiles.empty())
        {
            continue;
        }

        manifestFiles[1] = path.string() + manifestFiles[1];

        if (manifestFiles[0] == "VertexShader")
        {
            insertAt(LoadFileContent(manifestFiles[1]), IShader::kVertex);
        }
        else if (manifestFiles[0] == "FragmentShader")
        {
            insertAt(LoadFileContent(manifestFiles[1]), IShader::kFragment);
        }
        else if (manifestFiles[0] == "GeometryShader")
        {
            insertAt(LoadFileContent(manifestFiles[1]), IShader::kGeometry);
        }
        else if (manifestFiles[0] == "TesselationEvaluationShader")
        {
            insertAt(LoadFileContent(manifestFiles[1]), IShader::kTesselationEvaluateShader);
        }
        else if (manifestFiles[0] == "TesselationControlShader")
        {
            insertAt(LoadFileContent(manifestFiles[1]), IShader::kTesselationControlShader);
        }
    }

    ASSERT(shaderSources.size() < IShader::kCount);

    std::shared_ptr<IShader> shader = IShader::CreateFromSource(shaderSources);
    m_Shaders[filePath] = shader;
    return shader;
}

std::shared_ptr<ITexture2D> ResourceManagerImpl::GetTexture2D(const std::string& filePath)
{
    auto it = m_Textures2D.find(filePath);

    if (it == m_Textures2D.end())
    {
        return LoadTexture2D(filePath);
    }

    return it->second;
}

std::shared_ptr<ITexture2D> ResourceManagerImpl::LoadTexture2D(const std::string& filePath)
{
    std::shared_ptr<ITexture2D> texture = ITexture2D::LoadFromFile(filePath);
    m_Textures2D[filePath] = texture;

    return texture;
}

void ResourceManagerImpl::AddTexture2D(const std::string& path, const std::shared_ptr<ITexture2D>& texture)
{
    m_Textures2D[path] = texture;
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
