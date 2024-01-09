#include "material.h"
#include "renderer.h"

#include <algorithm>

namespace {
    constexpr std::string_view kMaterialTag = "u_material.";
}

Material::Material(const std::shared_ptr<Shader>& shader) :
    shader_{shader} {
    // retrieve all uniforms information from shader
    std::vector<UniformInfo> uniforms_info = std::move(shader->GetUniformInfos());

    for (const UniformInfo& info : uniforms_info) {
        TryAddNewProperty(info);
    }
}

std::int32_t Material::GetIntProperty(const char* name) const {
    return GetParam(name).GetInt();
}

void Material::SetIntProperty(const char* name, std::int32_t value) {
    GetParam(name).SetInt(value);
}

float Material::GetFloatProperty(const char* name) const {
    return GetParam(name).GetFloat();
}

void Material::SetFloatProperty(const char* name, float value) {
    GetParam(name).SetFloat(value);
}

glm::vec2 Material::GetVector2Property(const char* name) const {
    return GetParam(name).GetVector2();
}

void Material::SetVector2Property(const char* name, glm::vec2 value) {
    GetParam(name).SetVector2(value);
}

glm::vec3 Material::GetVector3Property(const char* name) const {
    return GetParam(name).GetVector3();
}

void Material::SetVector3Property(const char* name, glm::vec3 value) {
    GetParam(name).SetVector3(value);
}

glm::vec4 Material::GetVector4Property(const char* name) const {
    return GetParam(name).GetVector4();
}

void Material::SetVector4Property(const char* name, glm::vec4 value) {
    GetParam(name).SetVector4(value);
}

std::shared_ptr<Texture> Material::GetTextureProperty(const char* name) const {
    return GetParam(name).GetTexture();
}

void Material::SetTextureProperty(const char* name, const std::shared_ptr<Texture>& value) {
    GetParam(name).SetTexture(value);
}

void Material::TryAddNewProperty(const UniformInfo& info) {
    bool is_material_uniform = ContainsString(info.name, kMaterialTag.data());

    if (is_material_uniform) {
        AddNewProperty(info);
    }
}

void Material::AddNewProperty(const UniformInfo& info) {
    switch (info.uniform_type) {
    case UniformType::kVec4:
    {
        material_params_.try_emplace(info.name.substr(kMaterialTag.length()),
            info.name.c_str(), glm::vec4{0, 0, 0, 1});
        break;
    }
    case UniformType::kVec3:
    {
        material_params_.try_emplace(info.name.substr(kMaterialTag.length()),
            info.name.c_str(), glm::vec3{0, 0, 0});
        break;
    }
    case UniformType::kVec2:
    {
        material_params_.try_emplace(info.name.substr(kMaterialTag.length()),
            info.name.c_str(), glm::vec2{0, 0});
        break;
    }
    case UniformType::kFloat:
    {
        material_params_.try_emplace(info.name.substr(kMaterialTag.length()),
            info.name.c_str(), 0.0f);
        break;
    }
    case UniformType::kInt:
    {
        material_params_.try_emplace(info.name.substr(kMaterialTag.length()),
            info.name.c_str(), 0);
        break;
    }
    case UniformType::kSampler2D:
    {
        MaterialParam param{info.name.c_str(), Renderer::GetDefaultTexture(), num_texture_units_++};
        material_params_.try_emplace(info.name.substr(kMaterialTag.length()), param);
        break;
    }
    }
}

void Material::SetupRenderState() const {
    RenderCommand::SetWireframe(use_wireframe);
    RenderCommand::SetCullFace(cull_faces);
}

void Material::SetShaderUniforms() const {
    Shader& shader = GetShader();

    for (auto& [name, param] : material_params_) {
        param.SetUniform(shader);
    }
}