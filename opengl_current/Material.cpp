#include "Material.h"

#include "Renderer.h"

#include <algorithm>

namespace {
    constexpr std::string_view MaterialTag = "u_Material.";
}

using TextureSetter = PropertySetter<std::shared_ptr<Texture>>;

Material::Material(const std::shared_ptr<Shader>& shader) :
    shader_{ shader } {
    // retrieve all uniforms information from shader
    std::vector<UniformInfo> uniforms_info = std::move(shader->GetUniformInfos());

    for (const UniformInfo& info : uniforms_info) {
        TryAddNewProperty(info);
    }
}

std::int32_t Material::GetIntProperty(const char* name) const {
    return ints_.GetValue(name);
}

void Material::SetIntProperty(const char* name, std::int32_t value) {
    ints_.SetValue(name, value);
}

float Material::GetFloatProperty(const char* name) const {
    return floats_.GetValue(name);
}

void Material::SetFloatProperty(const char* name, float value) {
    floats_.SetValue(name, value);
}

glm::vec2 Material::GetVector2Property(const char* name) const {
    return vectors2_.GetValue(name);
}

void Material::SetVector2Property(const char* name, glm::vec2 value) {
    vectors2_.SetValue(name, value);
}

glm::vec3 Material::GetVector3Property(const char* name) const {
    return vectors3_.GetValue(name);
}

void Material::SetVector3Property(const char* name, glm::vec3 value) {
    vectors3_.SetValue(name, value);
}

glm::vec4 Material::GetVector4Property(const char* name) const {
    return vectors4_.GetValue(name);
}

void Material::SetVector4Property(const char* name, glm::vec4 value) {
    vectors4_.SetValue(name, value);
}

std::shared_ptr<Texture> Material::GetTextureProperty(const char* name) const {
    return textures_.GetValue(name);
}

void Material::SetTextureProperty(const char* name, const std::shared_ptr<Texture>& value) {
    textures_.SetValue(name, value);
}

void Material::TryAddNewProperty(const UniformInfo& info) {
    bool is_material_uniform = ContainsString(info.name, MaterialTag.data());

    if (is_material_uniform) {
        AddNewProperty(info);
    }
}

void Material::AddNewProperty(const UniformInfo& info) {
    switch (info.vertex_type) {
    case UniformType::kVec4:
    {
        AddNewVec4(info);
        break;
    }
    case UniformType::kVec3:
    {
        AddNewVec3(info);
        break;
    }
    case UniformType::kVec2:
    {
        AddNewVec2(info);
        break;
    }
    case UniformType::kFloat:
    {
        AddNewFloat(info);
        break;
    }
    case UniformType::kInt:
    {
        AddNewInt(info);
        break;
    }
    case UniformType::kSampler2D:
    {
        AddNewTexture(info);
        break;
    }
    }
}

void Material::AddNewTexture(const UniformInfo& info) {
    textures_.Add(Renderer::GetDefaultTexture(),
        info.name, info.name.substr(MaterialTag.length()));
}

void Material::AddNewInt(const UniformInfo& info) {
    ints_.Add(0, info.name, info.name.substr(MaterialTag.length()));
}

void Material::AddNewFloat(const UniformInfo& info) {
    floats_.Add(0.0f, info.name, info.name.substr(MaterialTag.length()));
}

void Material::AddNewVec2(const UniformInfo& info) {
    vectors2_.Add(glm::vec2{ 0, 0 }, info.name,
        info.name.substr(MaterialTag.length()));
}

void Material::AddNewVec3(const UniformInfo& info) {
    vectors3_.Add(glm::vec3{ 0.0f, 0.0f, 0.0f }, info.name,
        info.name.substr(MaterialTag.length()));
}

void Material::AddNewVec4(const UniformInfo& info) {
    vectors4_.Add(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
        info.name, info.name.substr(MaterialTag.length()));
}

void Material::SetupRenderState() const {
    RenderCommand::SetWireframe(use_wireframe);
    RenderCommand::SetCullFace(should_cull_faces);
}

void Material::SetShaderUniforms() const {
    Shader& shader = GetShader();

    floats_.RefreshVars(shader);
    ints_.RefreshVars(shader);

    vectors2_.RefreshVars(shader);
    vectors3_.RefreshVars(shader);
    vectors4_.RefreshVars(shader);

    TextureSetter::texture_unit = 0;
    textures_.RefreshVars(shader);
}