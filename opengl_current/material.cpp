#include "material.h"
#include "renderer.h"

#include <algorithm>

namespace
{
    constexpr std::string_view MaterialTag = "u_Material.";
}

Material::Material(const std::shared_ptr<Shader>& shader) :
    m_Shader{shader}
{
    // retrieve all uniforms information from shader
    std::vector<UniformInfo> uniformsInfo = std::move(shader->GetUniformsInfo());

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

std::shared_ptr<Texture> Material::GetTextureProperty(const char* name) const
{
    return GetParam(name).GetTexture();
}

void Material::SetTextureProperty(const char* name, const std::shared_ptr<Texture>& value)
{
    GetParam(name).SetTexture(value);
}

void Material::VisitForEachParam(IMaterialParameterVisitor& visitor)
{
    for (auto& [name, param] : m_MaterialParams)
    {
        param.Accept(visitor, name);
    }
}

void Material::TryAddNewProperty(const UniformInfo& info)
{
    bool bIsMaterialUniform = ContainsString(info.Name, MaterialTag);

    if (bIsMaterialUniform)
    {
        AddNewProperty(info);
    }
}

void Material::AddNewProperty(const UniformInfo& info)
{
    switch (info.Type)
    {
    case UniformType::Vec4:
    {
        m_MaterialParams.try_emplace(info.Name.substr(MaterialTag.length()),
            info.Name.c_str(), glm::vec4{0, 0, 0, 1});
        break;
    }
    case UniformType::Vec3:
    {
        m_MaterialParams.try_emplace(info.Name.substr(MaterialTag.length()),
            info.Name.c_str(), glm::vec3{0, 0, 0});
        break;
    }
    case UniformType::Vec2:
    {
        m_MaterialParams.try_emplace(info.Name.substr(MaterialTag.length()),
            info.Name.c_str(), glm::vec2{0, 0});
        break;
    }
    case UniformType::Float:
    {
        m_MaterialParams.try_emplace(info.Name.substr(MaterialTag.length()),
            info.Name.c_str(), 0.0f);
        break;
    }
    case UniformType::Int:
    {
        m_MaterialParams.try_emplace(info.Name.substr(MaterialTag.length()),
            info.Name.c_str(), 0);
        break;
    }
    case UniformType::Sampler2D:
    {
        MaterialParam param{info.Name.c_str(), Renderer::GetDefaultTexture(), m_NumTextureUnits++};
        m_MaterialParams.try_emplace(info.Name.substr(MaterialTag.length()), param);
        break;
    }
    }
}

void Material::SetupRenderState() const
{
    RenderCommand::SetCullFace(bCullFaces);
}

void Material::SetShaderUniforms() const
{
    std::shared_ptr<Shader> shader = GetShader();

    for (auto& [name, param] : m_MaterialParams)
    {
        param.SetUniform(*shader);
    }
}