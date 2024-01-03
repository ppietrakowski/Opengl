#include "material_parameter.h"
#include "error_macros.h"
#include "renderer.h"

#include <cstring>

MaterialParam::MaterialParam(const char* uniformName) :
    ParamType{MaterialParamType::kUnknown}
{
    strncpy(this->UniformName, uniformName, sizeof(this->UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, int32_t value) :
    ParamType{MaterialParamType::kInt}
{
    ParamValue.IntValue = value;

    strncpy(this->UniformName, uniformName, sizeof(this->UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, float value) :
    ParamType{MaterialParamType::kFloat}
{
    ParamValue.FloatValue = value;

    strncpy(this->UniformName, uniformName, sizeof(this->UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec2 value) :
    ParamType{MaterialParamType::kVec2}
{
    ParamValue.Vec2Value = value;

    strncpy(this->UniformName, uniformName, sizeof(this->UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec3 value) :
    ParamType{MaterialParamType::kVec3}
{
    ParamValue.Vec3Value = value;

    strncpy(this->UniformName, uniformName, sizeof(this->UniformName));
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec4 value) :
    ParamType{MaterialParamType::kVec2}
{
    ParamValue.Vec4Value = value;

    strncpy(this->UniformName, uniformName, sizeof(this->UniformName));
}

void MaterialParam::SetUniform(IShader& shader) const
{

    switch (ParamType)
    {
    case MaterialParamType::kInt:
        shader.SetUniformInt(UniformName, ParamValue.IntValue);
        break;
    case MaterialParamType::kFloat:
        shader.SetUniformFloat(UniformName, ParamValue.FloatValue);
        break;
    case MaterialParamType::kVec2:
        shader.SetUniformVec2(UniformName, ParamValue.Vec2Value);
        break;
    case MaterialParamType::kVec3:
        shader.SetUniformVec3(UniformName, ParamValue.Vec3Value);
        break;
    case MaterialParamType::kVec4:
        shader.SetUniformVec4(UniformName, ParamValue.Vec4Value);
        break;
    case MaterialParamType::kSampler2D:
        shader.SetSamplerUniform(UniformName, Texture, TextureUnit);
        break;
    default:
        break;
    }
}

int32_t MaterialParam::GetInt() const
{
    return ParamValue.IntValue;
}

float MaterialParam::GetFloat() const
{
    return ParamValue.FloatValue;
}

glm::vec2 MaterialParam::GetVector2() const
{
    return ParamValue.Vec2Value;
}

glm::vec3 MaterialParam::GetVector3() const
{
    return ParamValue.Vec3Value;
}

glm::vec4 MaterialParam::GetVector4() const
{
    return ParamValue.Vec4Value;
}

std::shared_ptr<ITexture> MaterialParam::GetTexture() const
{
    return Texture;
}

void MaterialParam::SetInt(int32_t value)
{
    ParamValue.IntValue = value;
}

void MaterialParam::SetFloat(float value)
{
    ParamValue.FloatValue = value;
}

void MaterialParam::SetVector2(glm::vec2 value)
{
    ParamValue.Vec2Value = value;
}

void MaterialParam::SetVector3(glm::vec3 value)
{
    ParamValue.Vec3Value = value;
}

void MaterialParam::SetVector4(glm::vec4 value)
{
    ParamValue.Vec4Value = value;
}

void MaterialParam::SetTexture(const std::shared_ptr<ITexture>& value)
{
    Texture = value;
}

