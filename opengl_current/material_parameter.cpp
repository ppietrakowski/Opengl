#include "material_parameter.h"
#include "error_macros.h"
#include "renderer.h"

#include <cstring>

MaterialParam::MaterialParam(const char* uniformName, std::int32_t value) :
    m_Param{TPrimitiveParameter<std::int32_t>{uniformName, value}} {
    m_ParamType = MaterialParamType::kInt;
    m_Getter = [](const ParamVariant& value) -> Parameter* {
        return const_cast<TPrimitiveParameter<std::int32_t>*>(&std::get<TPrimitiveParameter<std::int32_t>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, float value) :
    m_Param{TPrimitiveParameter<float>{uniformName, value}} {
    m_ParamType = MaterialParamType::kFloat;
    m_Getter = [](const ParamVariant& value) -> Parameter* {
        return const_cast<TPrimitiveParameter<float>*>(&std::get<TPrimitiveParameter<float>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec2 value) :
    m_Param{TPrimitiveParameter<glm::vec2>{uniformName, value}} {
    m_ParamType = MaterialParamType::kVec2;
    m_Getter = [](const ParamVariant& value) -> Parameter* {
        return const_cast<TPrimitiveParameter<glm::vec2>*>(&std::get<TPrimitiveParameter<glm::vec2>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec3 value) :
    m_Param{TPrimitiveParameter<glm::vec3>{uniformName, value}} {
    m_ParamType = MaterialParamType::kVec3;
    m_Getter = [](const ParamVariant& value) -> Parameter* {
        return const_cast<TPrimitiveParameter<glm::vec3>*>(&std::get<TPrimitiveParameter<glm::vec3>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, glm::vec4 value) :
    m_Param{TPrimitiveParameter<glm::vec4>{uniformName, value}} {
    m_ParamType = MaterialParamType::kVec4;
    m_Getter = [](const ParamVariant& value) -> Parameter* {
        return const_cast<TPrimitiveParameter<glm::vec4>*>(&std::get<TPrimitiveParameter<glm::vec4>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniformName, std::shared_ptr<Texture> value, std::uint32_t textureUnit) :
    m_Param{TextureParameter{uniformName, value, textureUnit}} {
    m_ParamType = MaterialParamType::kSampler2D;
    m_Getter = [](const ParamVariant& value) -> Parameter* { return const_cast<TextureParameter*>(&std::get<TextureParameter>(value)); };
}

void MaterialParam::SetUniform(Shader& shader) const {
    ASSERT(m_ParamType != MaterialParamType::kUnknown);
    m_Getter(m_Param)->SetUniform(shader);
}

std::int32_t MaterialParam::GetInt() const {
    return std::any_cast<std::int32_t>(m_Getter(m_Param)->GetValue());
}

float MaterialParam::GetFloat() const {
    return std::any_cast<float>(m_Getter(m_Param)->GetValue());
}

glm::vec2 MaterialParam::GetVector2() const {
    return std::any_cast<glm::vec2>(m_Getter(m_Param)->GetValue());
}

glm::vec3 MaterialParam::GetVector3() const {
    return std::any_cast<glm::vec3>(m_Getter(m_Param)->GetValue());
}

glm::vec4 MaterialParam::GetVector4() const {
    return std::any_cast<glm::vec4>(m_Getter(m_Param)->GetValue());
}

std::shared_ptr<Texture> MaterialParam::GetTexture() const {
    return std::any_cast<std::shared_ptr<Texture>>(m_Getter(m_Param)->GetValue());
}

void MaterialParam::SetInt(std::int32_t value) {
    m_Getter(m_Param)->SetValue(value);
}

void MaterialParam::SetFloat(float value) {
    m_Getter(m_Param)->SetValue(value);
}

void MaterialParam::SetVector2(glm::vec2 value) {
    m_Getter(m_Param)->SetValue(value);
}

void MaterialParam::SetVector3(glm::vec3 value) {
    m_Getter(m_Param)->SetValue(value);
}

void MaterialParam::SetVector4(glm::vec4 value) {
    m_Getter(m_Param)->SetValue(value);
}

void MaterialParam::SetTexture(const std::shared_ptr<Texture>& value) {
    m_Getter(m_Param)->SetValue(value);
}

void TextureParameter::SetUniform(Shader& shader) const {
    shader.SetSamplerUniform(m_UniformName.c_str(), m_Value, m_Texture);
}

void UnknownParameter::SetUniform(Shader& shader) const {
}
