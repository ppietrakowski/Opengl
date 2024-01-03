#include "material.h"
#include "renderer.h"

#include <algorithm>

namespace
{
    constexpr std::string_view kMaterialTag = "u_material.";
}

Material::Material(const std::shared_ptr<IShader>& shader) :
    Shader{shader}
{
    // retrieve all uniforms information from shader
    std::vector<UniformInfo> uniformsInfo = std::move(shader->GetUniformInfos());

    for (const UniformInfo& info : uniformsInfo)
    {
        TryAddNewProperty(info);
    }
}

int32_t Material::GetIntProperty(const char* name) const
{
    return GetParam(name).GetInt();
}

void Material::SetIntProperty(const char* name, int32_t value)
{
    GetParam(name).SetInt(value);
}

float Material::GetFloatProperty(const char* name) const
{
    return GetParam(name).GetFloat();
}

void Material::SetFloatProperty(const char* name, float value)
{
    GetParam(name).SetFloat(value);
}

glm::vec2 Material::GetVector2Property(const char* name) const
{
    return GetParam(name).GetVector2();
}

void Material::SetVector2Property(const char* name, glm::vec2 value)
{
    GetParam(name).SetVector2(value);
}

glm::vec3 Material::GetVector3Property(const char* name) const
{
    return GetParam(name).GetVector3();
}

void Material::SetVector3Property(const char* name, glm::vec3 value)
{
    GetParam(name).SetVector3(value);
}

glm::vec4 Material::GetVector4Property(const char* name) const
{
    return GetParam(name).GetVector4();
}

void Material::SetVector4Property(const char* name, glm::vec4 value)
{
    GetParam(name).SetVector4(value);
}

std::shared_ptr<ITexture> Material::GetTextureProperty(const char* name) const
{
    return GetParam(name).GetTexture();
}

void Material::SetTextureProperty(const char* name, const std::shared_ptr<ITexture>& value)
{
    GetParam(name).SetTexture(value);
}

void Material::TryAddNewProperty(const UniformInfo& info)
{
    bool bIsMaterialUniform = ContainsString(info.Name, kMaterialTag.data());

    if (bIsMaterialUniform)
    {
        AddNewProperty(info);
    }
}

void Material::AddNewProperty(const UniformInfo& info)
{
    switch (info.Type)
    {
    case UniformType::kVec4:
    {
        MaterialParams.try_emplace(info.Name.substr(kMaterialTag.length()),
            info.Name.c_str(), glm::vec4{0, 0, 0, 1});
        break;
    }
    case UniformType::kVec3:
    {
        MaterialParams.try_emplace(info.Name.substr(kMaterialTag.length()),
            info.Name.c_str(), glm::vec3{0, 0, 0});
        break;
    }
    case UniformType::kVec2:
    {
        MaterialParams.try_emplace(info.Name.substr(kMaterialTag.length()),
            info.Name.c_str(), glm::vec2{0, 0});
        break;
    }
    case UniformType::kFloat:
    {
        MaterialParams.try_emplace(info.Name.substr(kMaterialTag.length()),
            info.Name.c_str(), 0.0f);
        break;
    }
    case UniformType::kInt:
    {
        MaterialParams.try_emplace(info.Name.substr(kMaterialTag.length()),
            info.Name.c_str(), 0);
        break;
    }
    case UniformType::kSampler2D:
    {
        MaterialParam param{info.Name.c_str()};
        param.Texture = Renderer::GetDefaultTexture();
        param.TextureUnit = NumTextureUnits++;

        MaterialParams.try_emplace(info.Name.substr(kMaterialTag.length()), param);
        break;
    }
    }
}

void Material::SetupRenderState() const
{
    RenderCommand::SetWireframe(bUseWireframe);
    RenderCommand::SetCullFace(bCullFaces);
}

void Material::SetShaderUniforms() const
{
    IShader& shader = GetShader();

    for (auto& [name, param] : MaterialParams)
    {
        param.SetUniform(shader);
    }
}