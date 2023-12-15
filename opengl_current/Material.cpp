#include "Material.h"

#include "Renderer.h"

#include <algorithm>

namespace
{
    constexpr std::string_view MaterialTag = "u_Material.";
}

Material::Material(const std::shared_ptr<Shader>& shader) :
    _shader{ shader }
{
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
    if (ContainsString(info.Name, MaterialTag.data()))
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
    Property<std::shared_ptr<Texture>> property;
    strncpy(property.UniformName, info.Name.c_str(), 96);
    property.UniformName[95] = 0;
    property.Value = Renderer::GetDefaultTexture();
    _textures.Add(property, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewInt(const UniformInfo& info)
{
    Property<std::int32_t> property;
    strncpy(property.UniformName, info.Name.c_str(), 96);
    property.UniformName[95] = 0;
    property.Value = 0;
    _ints.Add(property, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewFloat(const UniformInfo& info)
{
    Property<float> property;
    strncpy(property.UniformName, info.Name.c_str(), 96);
    property.UniformName[95] = 0;
    property.Value = 0.0f;
    _floats.Add(property, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewVec2(const UniformInfo& info)
{
    Property<glm::vec2> property;
    strncpy(property.UniformName, info.Name.c_str(), 96);
    property.UniformName[95] = 0;
    property.Value = glm::vec2{ 0.0f, 0.0f };
    _vectors2.Add(property, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewVec3(const UniformInfo& info)
{
    Property<glm::vec3> property;
    strncpy(property.UniformName, info.Name.c_str(), 96);
    property.UniformName[95] = 0;
    property.Value = glm::vec3{ 0.0f, 0.0f, 0.0f };
    _vectors3.Add(property, info.Name.substr(MaterialTag.length()));
}

void Material::AddNewVec4(const UniformInfo& info)
{
    Property<glm::vec4> property;
    strncpy(property.UniformName, info.Name.c_str(), 96);
    property.UniformName[95] = 0;
    property.Value = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f };
    _vectors4.Add(property, info.Name.substr(MaterialTag.length()));
}

void Material::SetupRenderState() const
{
    RenderCommand::SetWireframe(UseWireframe);
    RenderCommand::SetCullFace(ShouldCullFaces);
}

void Material::SetShaderUniforms() const
{
    Shader& shader = *this->_shader;

    _floats.RefreshVars(shader);
    _ints.RefreshVars(shader);

    _vectors2.RefreshVars(shader);
    _vectors3.RefreshVars(shader);
    _vectors4.RefreshVars(shader);

    PropertySetter<std::shared_ptr<Texture>>::textureUnit = 0;
    _textures.RefreshVars(shader);
}