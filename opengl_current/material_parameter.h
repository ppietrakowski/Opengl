#pragma once

#include "texture.h"
#include "shader.h"

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include <cstring>
#include <array>

enum class MaterialParamType : int8_t {
    kUnknown = 0,
    kInt,
    kFloat,
    kVec2,
    kVec3,
    kVec4,
    kSampler2D,
    kNumMaterialParamsType
};

union ParamVal {
    int32_t int_value;
    float float_value;
    glm::vec2 vec2_value;
    glm::vec3 vec3_value;
    glm::vec4 vec4_value;
};

class MaterialParam {
    friend class Material;

public:
    MaterialParam() = default;
    MaterialParam(const MaterialParam&) = default;
    MaterialParam(const char* uniform_name);
    MaterialParam(const char* uniform_name, int32_t value);
    MaterialParam(const char* uniform_name, float value);
    MaterialParam(const char* uniform_name, glm::vec2 value);
    MaterialParam(const char* uniform_name, glm::vec3 value);
    MaterialParam(const char* uniform_name, glm::vec4 value);

    MaterialParam& operator=(const MaterialParam&) = default;
    void SetUniform(Shader& shader) const;

    int32_t GetInt() const;
    float GetFloat() const;
    glm::vec2 GetVector2() const;
    glm::vec3 GetVector3() const;
    glm::vec4 GetVector4() const;

    std::shared_ptr<Texture> GetTexture() const;

    void SetInt(int32_t value);
    void SetFloat(float value);
    void SetVector2(glm::vec2 value);
    void SetVector3(glm::vec3 value);
    void SetVector4(glm::vec4 value);
    void SetTexture(const std::shared_ptr<Texture>& value);

    uint32_t texture_unit{ 0 };

private:
    ParamVal param_value_;
    std::shared_ptr<Texture> texture_;
    MaterialParamType param_type_;
    char uniform_name_[64];
};