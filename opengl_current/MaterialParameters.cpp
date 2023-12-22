#include "MaterialParameters.h"
#include "ErrorMacros.h"
#include "Renderer.h"

#include <cstring>

MaterialParam::MaterialParam(const char* uniform_name) :
    param_type_{ MaterialParamType::kUnknown } {
    strncpy(this->uniform_name_, uniform_name, sizeof(this->uniform_name_));
}

MaterialParam::MaterialParam(const char* uniform_name, int32_t value) :
    param_type_{ MaterialParamType::kInt } {
    param_value_.int_value = value;

    strncpy(this->uniform_name_, uniform_name, sizeof(this->uniform_name_));
}

MaterialParam::MaterialParam(const char* uniform_name, float value) :
    param_type_{ MaterialParamType::kFloat } {
    param_value_.float_value = value;

    strncpy(this->uniform_name_, uniform_name, sizeof(this->uniform_name_));
}

MaterialParam::MaterialParam(const char* uniform_name, glm::vec2 value) :
    param_type_{ MaterialParamType::kVec2 } {
    param_value_.vec2_value = value;

    strncpy(this->uniform_name_, uniform_name, sizeof(this->uniform_name_));
}

MaterialParam::MaterialParam(const char* uniform_name, glm::vec3 value) :
    param_type_{ MaterialParamType::kVec3 } {
    param_value_.vec3_value = value;

    strncpy(this->uniform_name_, uniform_name, sizeof(this->uniform_name_));
}

MaterialParam::MaterialParam(const char* uniform_name, glm::vec4 value) :
    param_type_{ MaterialParamType::kVec2 } {
    param_value_.vec4_value = value;

    strncpy(this->uniform_name_, uniform_name, sizeof(this->uniform_name_));
}

void MaterialParam::SetUniform(Shader& shader) const {

    switch (param_type_) {
    case MaterialParamType::kInt:
        shader.SetUniformInt(uniform_name_, param_value_.int_value);
        break;
    case MaterialParamType::kFloat:
        shader.SetUniformFloat(uniform_name_, param_value_.float_value);
        break;
    case MaterialParamType::kVec2:
        shader.SetUniformVec2(uniform_name_, param_value_.vec2_value);
        break;
    case MaterialParamType::kVec3:
        shader.SetUniformVec3(uniform_name_, param_value_.vec3_value);
        break;
    case MaterialParamType::kVec4:
        shader.SetUniformVec4(uniform_name_, param_value_.vec4_value);
        break;
    case MaterialParamType::kSampler2D:
        shader.SetSamplerUniform(uniform_name_, texture_, texture_unit);
        break;
    default:
        break;
    }
}

int32_t MaterialParam::GetInt() const {
    return param_value_.int_value;
}

float MaterialParam::GetFloat() const {
    return param_value_.float_value;
}

glm::vec2 MaterialParam::GetVector2() const {
    return param_value_.vec2_value;
}

glm::vec3 MaterialParam::GetVector3() const {
    return param_value_.vec3_value;
}

glm::vec4 MaterialParam::GetVector4() const {
    return param_value_.vec4_value;
}

std::shared_ptr<Texture> MaterialParam::GetTexture() const {
    return texture_;
}

void MaterialParam::SetInt(int32_t value) {
    param_value_.int_value = value;
}

void MaterialParam::SetFloat(float value) {
    param_value_.float_value = value;
}

void MaterialParam::SetVector2(glm::vec2 value) {
    param_value_.vec2_value = value;
}

void MaterialParam::SetVector3(glm::vec3 value) {
    param_value_.vec3_value = value;
}

void MaterialParam::SetVector4(glm::vec4 value) {
    param_value_.vec4_value = value;
}

void MaterialParam::SetTexture(const std::shared_ptr<Texture>& value) {
    texture_ = value;
}

