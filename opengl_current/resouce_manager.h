#pragma once

#include <unordered_map>
#include <memory>
#include "skeletal_mesh.h"
#include "static_mesh.h"
#include "shader.h"
#include "texture.h"
#include "material.h"

class ResourceManagerImpl;

class ResourceManager
{
    friend class Level;
    friend class ResourceManagerImpl;
public:
    static std::shared_ptr<Shader> GetShader(const std::string& filePath);
    static std::shared_ptr<Texture2D> GetTexture2D(const std::string& filePath);
    static void AddTexture2D(const std::string& path, const std::shared_ptr<Texture2D>& texture);
    static std::shared_ptr<SkeletalMesh> GetSkeletalMesh(const std::string& filePath);
    static std::shared_ptr<StaticMesh> GetStaticMesh(const std::string& filePath);

    static std::shared_ptr<Material> GetMaterial(const std::string& materialName);
    static std::shared_ptr<Material> CreateMaterial(const std::string& shaderFilePath, const std::string& materialName);

    static void Quit();
private:
    static std::shared_ptr<ResourceManagerImpl> s_ResourceManagerInstance;

private:
    static std::shared_ptr<ResourceManagerImpl> CreateResourceManager();
};


