#include "material_parameter.h"
#include "error_macros.h"
#include "renderer.h"

#include <cstring>

MaterialParam::MaterialParam(const char* uniformName, int value) :
    m_Parameter{PrimitiveParameter<std::int32_t>{uniformName, value}}
{
    m_ParamType = MaterialParamType::Int;
    m_Getter = [](const ParamVariant& value) -> Parameter*
    {
        return const_cast<PrimitiveParameter<int>*>(&std::get<PrimitiveParameter<int>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, float value) :
    m_Parameter{PrimitiveParameter<float>{uniformName, value}}
{
    m_ParamType = MaterialParamType::Float;
    m_Getter = [](const ParamVariant& value) -> Parameter*
    {
        return const_cast<PrimitiveParameter<float>*>(&std::get<PrimitiveParameter<float>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec2 value) :
    m_Parameter{PrimitiveParameter<glm::vec2>{uniformName, value}}
{
    m_ParamType = MaterialParamType::Vec2;
    m_Getter = [](const ParamVariant& value) -> Parameter*
    {
        return const_cast<PrimitiveParameter<glm::vec2>*>(&std::get<PrimitiveParameter<glm::vec2>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec3 value) :
    m_Parameter{PrimitiveParameter<glm::vec3>{uniformName, value}}
{
    m_ParamType = MaterialParamType::Vec3;
    m_Getter = [](const ParamVariant& value) -> Parameter*
    {
        return const_cast<PrimitiveParameter<glm::vec3>*>(&std::get<PrimitiveParameter<glm::vec3>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec4 value) :
    m_Parameter{PrimitiveParameter<glm::vec4>{uniformName, value}}
{
    m_ParamType = MaterialParamType::Vec4;
    m_Getter = [](const ParamVariant& value) -> Parameter*
    {
        return const_cast<PrimitiveParameter<glm::vec4>*>(&std::get<PrimitiveParameter<glm::vec4>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, std::shared_ptr<Texture> value, uint32_t textureUnit) :
    m_Parameter{TextureParameter{uniformName, value, textureUnit}}
{
    m_ParamType = MaterialParamType::Sampler2D;
    m_Getter = [](const ParamVariant& value) -> Parameter*
    {
        return const_cast<TextureParameter*>(&std::get<TextureParameter>(value));
    };
}

void MaterialParam::SetUniform(Shader& shader) const
{
    ASSERT(m_ParamType != MaterialParamType::Unknown);
    m_Getter(m_Parameter)->SetUniform(shader);
}

int MaterialParam::GetInt() const
{
    return std::any_cast<int>(m_Getter(m_Parameter)->GetValue());
}

float MaterialParam::GetFloat() const
{
    return std::any_cast<float>(m_Getter(m_Parameter)->GetValue());
}

glm::vec2 MaterialParam::GetVector2() const
{
    return std::any_cast<glm::vec2>(m_Getter(m_Parameter)->GetValue());
}

glm::vec3 MaterialParam::GetVector3() const
{
    return std::any_cast<glm::vec3>(m_Getter(m_Parameter)->GetValue());
}

glm::vec4 MaterialParam::GetVector4() const
{
    return std::any_cast<glm::vec4>(m_Getter(m_Parameter)->GetValue());
}

std::shared_ptr<Texture> MaterialParam::GetTexture() const
{
    return std::any_cast<std::shared_ptr<Texture>>(m_Getter(m_Parameter)->GetValue());
}

void MaterialParam::SetInt(int value)
{
    m_Getter(m_Parameter)->SetValue(value);
}

void MaterialParam::SetFloat(float value)
{
    m_Getter(m_Parameter)->SetValue(value);
}

void MaterialParam::SetVector2(glm::vec2 value)
{
    m_Getter(m_Parameter)->SetValue(value);
}

void MaterialParam::SetVector3(glm::vec3 value)
{
    m_Getter(m_Parameter)->SetValue(value);
}

void MaterialParam::SetVector4(glm::vec4 value)
{
    m_Getter(m_Parameter)->SetValue(value);
}

void MaterialParam::SetTexture(const std::shared_ptr<Texture>& value)
{
    m_Getter(m_Parameter)->SetValue(value);
}

void MaterialParam::Accept(IMaterialParameterVisitor& visitor, const std::string& name)
{
    Parameter* paramater = m_Getter(m_Parameter);
    paramater->Accept(visitor, name);
}

void TextureParameter::SetUniform(Shader& shader) const
{
    m_Texture->Bind(m_TextureUnit);
    shader.SetSamplerUniform(m_UniformName.c_str(), m_Texture, m_TextureUnit);
}

void UnknownParameter::SetUniform(Shader& shader) const
{
}
