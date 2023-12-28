#include "material_parameter.h"
#include "error_macros.h"
#include "renderer.h"

#include <cstring>

MaterialParam::MaterialParam(const char* uniformName) :
    m_ParamType{MaterialParamType::kUnknown}
{
    strncpy(this->m_UniformName, uniformName, sizeof(this->m_UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, int32_t value) :
    m_ParamType{MaterialParamType::kInt}
{
    m_ParamValue.IntValue = value;

    strncpy(this->m_UniformName, uniformName, sizeof(this->m_UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, float value) :
    m_ParamType{MaterialParamType::kFloat}
{
    m_ParamValue.FloatValue = value;

    strncpy(this->m_UniformName, uniformName, sizeof(this->m_UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec2 value) :
    m_ParamType{MaterialParamType::kVec2}
{
    m_ParamValue.Vec2Value = value;

    strncpy(this->m_UniformName, uniformName, sizeof(this->m_UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec3 value) :
    m_ParamType{MaterialParamType::kVec3}
{
    m_ParamValue.Vec3Value = value;

    strncpy(this->m_UniformName, uniformName, sizeof(this->m_UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec4 value) :
    m_ParamType{MaterialParamType::kVec2}
{
    m_ParamValue.Vec4Value = value;

    strncpy(this->m_UniformName, uniformName, sizeof(this->m_UniformName));
}

void MaterialParam::SetUniform(IShader& shader) const
{

    switch (m_ParamType)
    {
    case MaterialParamType::kInt:
        shader.SetUniformInt(m_UniformName, m_ParamValue.IntValue);
        break;
    case MaterialParamType::kFloat:
        shader.SetUniformFloat(m_UniformName, m_ParamValue.FloatValue);
        break;
    case MaterialParamType::kVec2:
        shader.SetUniformVec2(m_UniformName, m_ParamValue.Vec2Value);
        break;
    case MaterialParamType::kVec3:
        shader.SetUniformVec3(m_UniformName, m_ParamValue.Vec3Value);
        break;
    case MaterialParamType::kVec4:
        shader.SetUniformVec4(m_UniformName, m_ParamValue.Vec4Value);
        break;
    case MaterialParamType::kSampler2D:
        shader.SetSamplerUniform(m_UniformName, m_Texture, TextureUnit);
        break;
    default:
        break;
    }
}

int32_t MaterialParam::GetInt() const
{
    return m_ParamValue.IntValue;
}

float MaterialParam::GetFloat() const
{
    return m_ParamValue.FloatValue;
}

glm::vec2 MaterialParam::GetVector2() const
{
    return m_ParamValue.Vec2Value;
}

glm::vec3 MaterialParam::GetVector3() const
{
    return m_ParamValue.Vec3Value;
}

glm::vec4 MaterialParam::GetVector4() const
{
    return m_ParamValue.Vec4Value;
}

std::shared_ptr<ITexture> MaterialParam::GetTexture() const
{
    return m_Texture;
}

void MaterialParam::SetInt(int32_t value)
{
    m_ParamValue.IntValue = value;
}

void MaterialParam::SetFloat(float value)
{
    m_ParamValue.FloatValue = value;
}

void MaterialParam::SetVector2(glm::vec2 value)
{
    m_ParamValue.Vec2Value = value;
}

void MaterialParam::SetVector3(glm::vec3 value)
{
    m_ParamValue.Vec3Value = value;
}

void MaterialParam::SetVector4(glm::vec4 value)
{
    m_ParamValue.Vec4Value = value;
}

void MaterialParam::SetTexture(const std::shared_ptr<ITexture>& value)
{
    m_Texture = value;
}

