#include "resouce_manager.h"
#include <fstream>
#include <filesystem>

std::shared_ptr<ResourceManagerImpl> ResourceManager::resource_manager_instance_;

class ResourceManagerImpl {
public:
    std::shared_ptr<Shader> GetShader(const std::string& file_path);
    std::shared_ptr<Shader> LoadShader(const std::string& file_path);

    std::shared_ptr<Texture2D> GetTexture2D(const std::string& file_path);
    std::shared_ptr<Texture2D> LoadTexture2D(const std::string& file_path);
    void AddTexture2D(const std::string& path, const std::shared_ptr<Texture2D>& texture);

    std::shared_ptr<SkeletalMesh> GetSkeletalMesh(const std::string& file_path);
    std::shared_ptr<SkeletalMesh> LoadSkeletalMesh(const std::string& file_path);

    std::shared_ptr<StaticMesh> GetStaticMesh(const std::string& file_path);
    std::shared_ptr<StaticMesh> LoadStaticMesh(const std::string& file_path);

    std::shared_ptr<Material> GetMaterial(const std::string& material_name);
    std::shared_ptr<Material> CreateMaterial(const std::string& shader_file_path, const std::string& material_name);

private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders_;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials_;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures2D_;
    std::unordered_map<std::string, std::shared_ptr<SkeletalMesh>> skeletal_meshes_;
    std::unordered_map<std::string, std::shared_ptr<StaticMesh>> static_meshes_;
};

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& file_path) {
    ASSERT(resource_manager_instance_);
    return resource_manager_instance_->GetShader(file_path);
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture2D(const std::string& file_path) {
    ASSERT(resource_manager_instance_);
    return resource_manager_instance_->GetTexture2D(file_path);
}

void ResourceManager::AddTexture2D(const std::string& path, const std::shared_ptr<Texture2D>& texture) {
    ASSERT(resource_manager_instance_);
    return resource_manager_instance_->AddTexture2D(path, texture);
}

std::shared_ptr<SkeletalMesh> ResourceManager::GetSkeletalMesh(const std::string& file_path) {
    ASSERT(resource_manager_instance_);
    return resource_manager_instance_->GetSkeletalMesh(file_path);
}

std::shared_ptr<StaticMesh> ResourceManager::GetStaticMesh(const std::string& file_path) {
    ASSERT(resource_manager_instance_);
    return resource_manager_instance_->GetStaticMesh(file_path);
}

std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& material_name) {
    ASSERT(resource_manager_instance_);
    return resource_manager_instance_->GetMaterial(material_name);
}

std::shared_ptr<Material> ResourceManager::CreateMaterial(const std::string& shader_file_path, const std::string& material_name) {
    ASSERT(resource_manager_instance_);
    return resource_manager_instance_->CreateMaterial(shader_file_path, material_name);
}

void ResourceManager::Quit() {
    resource_manager_instance_ = nullptr;
}

std::shared_ptr<ResourceManagerImpl> ResourceManager::CreateResourceManager() {
    resource_manager_instance_ = std::make_shared<ResourceManagerImpl>();
    return resource_manager_instance_;
}

std::shared_ptr<Shader> ResourceManagerImpl::GetShader(const std::string& file_path) {
    auto it = shaders_.find(file_path);

    if (it == shaders_.end()) {
        return LoadShader(file_path);
    }

    return it->second;
}

struct ShaderStringMatch {
    char tag[32];
    ShaderIndex::IndexType index;
};

std::shared_ptr<Shader> ResourceManagerImpl::LoadShader(const std::string& file_path) {
    std::string line;
    std::fstream file(file_path);
    std::vector<std::string> shader_sources;

    ASSERT(file.good() || !file.fail());

    shader_sources.reserve(ShaderIndex::kCount);
    std::filesystem::path path = file_path;
    path = path.remove_filename();

    auto insertAt = [&shader_sources](const std::string& source, std::int32_t index) {
        if (shader_sources.size() <= index) {
            std::int32_t new_size = index + 1;
            shader_sources.resize(new_size);
        }

        shader_sources[index] = source;
    };

    const ShaderStringMatch string_matches[] =
    {
        {"VertexShader", ShaderIndex::kVertex},
        {"FragmentShader", ShaderIndex::kFragment},
        {"GeometryShader", ShaderIndex::kGeometry},
        {"TesselationControlShader", ShaderIndex::kTesselationControlShader},
        {"TesselationEvaluationShader", ShaderIndex::kTesselationEvaluateShader},
    };

    while (std::getline(file, line)) {
        std::vector<std::string> manifest_files = SplitString(line, "=");

        if (manifest_files.empty()) {
            continue;
        }

        manifest_files[1] = path.string() + manifest_files[1];

        for (const ShaderStringMatch& match : string_matches) {
            if (manifest_files[0] == match.tag) {
                insertAt(LoadFileContent(manifest_files[1]), match.index);
                break;
            }
        }
    }

    ASSERT(shader_sources.size() < ShaderIndex::kCount);

    std::shared_ptr<Shader> shader = Shader::CreateFromSource(shader_sources);
    shaders_[file_path] = shader;
    return shader;
}

std::shared_ptr<Texture2D> ResourceManagerImpl::GetTexture2D(const std::string& file_path) {
    auto it = textures2D_.find(file_path);

    if (it == textures2D_.end()) {
        return LoadTexture2D(file_path);
    }

    return it->second;
}

std::shared_ptr<Texture2D> ResourceManagerImpl::LoadTexture2D(const std::string& file_path) {
    std::shared_ptr<Texture2D> texture = Texture2D::LoadFromFile(file_path);
    textures2D_[file_path] = texture;

    return texture;
}

void ResourceManagerImpl::AddTexture2D(const std::string& path, const std::shared_ptr<Texture2D>& texture) {
    textures2D_[path] = texture;
}

std::shared_ptr<SkeletalMesh> ResourceManagerImpl::GetSkeletalMesh(const std::string& file_path) {
    auto it = skeletal_meshes_.find(file_path);

    if (it == skeletal_meshes_.end()) {
        return LoadSkeletalMesh(file_path);
    }

    return it->second;
}

std::shared_ptr<SkeletalMesh> ResourceManagerImpl::LoadSkeletalMesh(const std::string& file_path) {
    std::shared_ptr<SkeletalMesh> skeletal_mesh = std::make_shared<SkeletalMesh>(file_path, GetMaterial("default"));
    skeletal_meshes_[file_path] = skeletal_mesh;
    return skeletal_mesh;
}

std::shared_ptr<StaticMesh> ResourceManagerImpl::GetStaticMesh(const std::string& file_path) {
    auto it = static_meshes_.find(file_path);

    if (it == static_meshes_.end()) {
        return LoadStaticMesh(file_path);
    }

    return it->second;
}

std::shared_ptr<StaticMesh> ResourceManagerImpl::LoadStaticMesh(const std::string& file_path) {
    std::shared_ptr<StaticMesh> static_mesh = std::make_shared<StaticMesh>(file_path, GetMaterial("default"));
    static_meshes_[file_path] = static_mesh;
    return static_mesh;
}

std::shared_ptr<Material> ResourceManagerImpl::GetMaterial(const std::string& material_name) {
    auto it = materials_.find(material_name);

    if (it == materials_.end()) {
        THROW_ERROR("Couldn't find material with such name");
    }

    return it->second;
}

std::shared_ptr<Material> ResourceManagerImpl::CreateMaterial(const std::string& shader_file_path, const std::string& material_name) {
    auto shader = GetShader(shader_file_path);
    materials_[material_name] = std::make_shared<Material>(shader);
    return materials_[material_name];
}
