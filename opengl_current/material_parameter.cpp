#include "material_parameter.h"
#include "error_macros.h"
#include "renderer.h"

#include <cstring>

MaterialParam::MaterialParam(const char* uniform_name, std::int32_t value) :
    param_{PrimitiveParameter<std::int32_t>{uniform_name, value}} {
    param_type_ = MaterialParamType::kInt;
    getter_ = [](const ParamVariant& value) -> Parameter* {
        return const_cast<PrimitiveParameter<std::int32_t>*>(&std::get<PrimitiveParameter<std::int32_t>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniform_name, float value) :
    param_{PrimitiveParameter<float>{uniform_name, value}} {
    param_type_ = MaterialParamType::kFloat;
    getter_ = [](const ParamVariant& value) -> Parameter* {
        return const_cast<PrimitiveParameter<float>*>(&std::get<PrimitiveParameter<float>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniform_name, glm::vec2 value) :
    param_{PrimitiveParameter<glm::vec2>{uniform_name, value}} {
    param_type_ = MaterialParamType::kVec2;
    getter_ = [](const ParamVariant& value) -> Parameter* {
        return const_cast<PrimitiveParameter<glm::vec2>*>(&std::get<PrimitiveParameter<glm::vec2>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniform_name, glm::vec3 value) :
    param_{PrimitiveParameter<glm::vec3>{uniform_name, value}} {
    param_type_ = MaterialParamType::kVec3;
    getter_ = [](const ParamVariant& value) -> Parameter* {
        return const_cast<PrimitiveParameter<glm::vec3>*>(&std::get<PrimitiveParameter<glm::vec3>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniform_name, glm::vec4 value) :
    param_{PrimitiveParameter<glm::vec4>{uniform_name, value}} {
    param_type_ = MaterialParamType::kVec4;
    getter_ = [](const ParamVariant& value) -> Parameter* {
        return const_cast<PrimitiveParameter<glm::vec4>*>(&std::get<PrimitiveParameter<glm::vec4>>(value));
    };
}

MaterialParam::MaterialParam(const char* uniform_name, std::shared_ptr<Texture> value, std::uint32_t texture_unit) :
    param_{TextureParameter{uniform_name, value, texture_unit}} {
    param_type_ = MaterialParamType::kSampler2D;
    getter_ = [](const ParamVariant& value) -> Parameter* { return const_cast<TextureParameter*>(&std::get<TextureParameter>(value)); };
}

void MaterialParam::SetUniform(Shader& shader) const {
    ASSERT(param_type_ != MaterialParamType::kUnknown);
    getter_(param_)->SetUniform(shader);
}

std::int32_t MaterialParam::GetInt() const {
    return std::any_cast<std::int32_t>(getter_(param_)->GetValue());
}

float MaterialParam::GetFloat() const {
    return std::any_cast<float>(getter_(param_)->GetValue());
}

glm::vec2 MaterialParam::GetVector2() const {
    return std::any_cast<glm::vec2>(getter_(param_)->GetValue());
}

glm::vec3 MaterialParam::GetVector3() const {
    return std::any_cast<glm::vec3>(getter_(param_)->GetValue());
}

glm::vec4 MaterialParam::GetVector4() const {
    return std::any_cast<glm::vec4>(getter_(param_)->GetValue());
}

std::shared_ptr<Texture> MaterialParam::GetTexture() const {
    return std::any_cast<std::shared_ptr<Texture>>(getter_(param_)->GetValue());
}

void MaterialParam::SetInt(std::int32_t value) {
    getter_(param_)->SetValue(value);
}

void MaterialParam::SetFloat(float value) {
    getter_(param_)->SetValue(value);
}

void MaterialParam::SetVector2(glm::vec2 value) {
    getter_(param_)->SetValue(value);
}

void MaterialParam::SetVector3(glm::vec3 value) {
    getter_(param_)->SetValue(value);
}

void MaterialParam::SetVector4(glm::vec4 value) {
    getter_(param_)->SetValue(value);
}

void MaterialParam::SetTexture(const std::shared_ptr<Texture>& value) {
    getter_(param_)->SetValue(value);
}

void TextureParameter::SetUniform(Shader& shader) const {
    shader.SetSamplerUniform(uniform_name_.c_str(), texture_, texture_unit_);
}

void UnknownParameter::SetUniform(Shader& shader) const {
}
