#include "Material.h"

#include "Renderer.h"

#include <algorithm>

namespace
{
    constexpr std::string_view MaterialTag = "u_Material.";
}

using TextureSetter = PropertySetter<std::shared_ptr<Texture>>;

Material::Material(const std::shared_ptr<Shader>& shader) :
    _shader{ shader }
{
    // retrieve all uniforms information from shader
    std::vector<UniformInfo> uniformInfos;
    shader->GetUniformInfos(uniformInfos);

    for (const UniformInfo& info : uniformInfos)
    {
        TryAddNewProperty(info);
    }
}

bool Material::GetIntProperty(const char* name, std::int32_t& outInt) const
{
    return _ints.GetValue(name, outInt);
}

void Material::SetIntProperty(const char* name, std::int32_t Value)
{
    _ints.SetValue(name, Value);
}

bool Material::GetFloatProperty(const char* name, float& outFloat) const
{
    return _floats.GetValue(name, outFloat);
}

void Material::SetFloatProperty(const char* name, float Value)
{
    _floats.SetValue(name, Value);
}

bool Material::GetVector2Property(const char* name, glm::vec2& outVec) const
{
    return _vectors2.GetValue(name, outVec);
}

void Material::SetVector2Property(const char* name, glm::vec2 Value)
{
    _vectors2.SetValue(name, Value);
}

bool Material::GetVector3Property(const char* name, glm::vec3& outVec) const
{
    return _vectors3.GetValue(name, outVec);
}

void Material::SetVector3Property(const char* name, glm::vec3 Value)
{
    _vectors3.SetValue(name, Value);
}

bool Material::GetVector4Property(const char* name, glm::vec4& outVec) const
{
    return _vectors4.GetValue(name, outVec);
}

void Material::SetVector4Property(const char* name, glm::vec4 Value)
{
    _vectors4.SetValue(name, Value);
}

bool Material::GetTextureProperty(const char* name, std::shared_ptr<Texture>& outTexture) const
{
    return _textures.GetValue(name, outTexture);
}

void Material::SetTextureProperty(const char* name, const std::shared_ptr<Texture>& Value)
{
    _textures.SetValue(name, Value);
}

void Material::TryAddNewProperty(const UniformInfo& info)
{
    bool isMaterialUniform = ContainsString(info.Name, MaterialTag.data());

    if (isMaterialUniform)
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
        AddNewVec4(info);
        break;
    }
    case UniformType::Vec3:
    {
        AddNewVec3(info);
        break;
    }
    case UniformType::Vec2:
    {
        AddNewVec2(info);
        break;
    }
    case UniformType::Float:
    {
        AddNewFloat(info);
        break;
    }
    case UniformType::Int:
    {
        AddNewInt(info);
        break;
    }
    case UniformType::Sampler2D:
    {
        AddNewTexture(info);
        break;
    }
    }
}

void Material::AddNewTexture(const UniformInfo& info)
{
    _textures.Add(Renderer::GetDefaultTexture(),
        info.Name, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewInt(const UniformInfo& info)
{
    _ints.Add(0, info.Name, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewFloat(const UniformInfo& info)
{
    _floats.Add(0.0f, info.Name, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewVec2(const UniformInfo& info)
{
    _vectors2.Add(glm::vec2{ 0, 0 }, info.Name,
        info.Name.substr(MaterialTag.length()));
}

void Material::AddNewVec3(const UniformInfo& info)
{
    _vectors3.Add(glm::vec3{ 0.0f, 0.0f, 0.0f }, info.Name,
        info.Name.substr(MaterialTag.length()));
}

void Material::AddNewVec4(const UniformInfo& info)
{
    _vectors4.Add(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
        info.Name, info.Name.substr(MaterialTag.length()));
}

void Material::SetupRenderState() const
{
    RenderCommand::SetWireframe(UseWireframe);
    RenderCommand::SetCullFace(ShouldCullFaces);
}

void Material::SetShaderUniforms() const
{
    Shader& shader = GetShader();

    _floats.RefreshVars(shader);
    _ints.RefreshVars(shader);

    _vectors2.RefreshVars(shader);
    _vectors3.RefreshVars(shader);
    _vectors4.RefreshVars(shader);

    TextureSetter::TextureUnit = 0;
    _textures.RefreshVars(shader);
}