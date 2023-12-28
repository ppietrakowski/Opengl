#pragma once

#include "texture.h"
#include "shader.h"

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include <cstring>
#include <array>

enum class MaterialParamType : int8_t
{
    kUnknown = 0,
    kInt,
    kFloat,
    kVec2,
    kVec3,
    kVec4,
    kSampler2D,
    kNumMaterialParamsType
};

union ParamVal
{
    int32_t IntValue;
    float FloatValue;
    glm::vec2 Vec2Value;
    glm::vec3 Vec3Value;
    glm::vec4 Vec4Value;
};

class MaterialParam
{
    friend class Material;

public:
    MaterialParam() = default;
    MaterialParam(const MaterialParam&) = default;
    MaterialParam(const char* uniformName);
    MaterialParam(const char* uniformName, int32_t value);
    MaterialParam(const char* uniformName, float value);
    MaterialParam(const char* uniformName, glm::vec2 value);
    MaterialParam(const char* uniformName, glm::vec3 value);
    MaterialParam(const char* uniformName, glm::vec4 value);

    MaterialParam& operator=(const MaterialParam&) = default;
    void SetUniform(IShader& shader) const;

    int32_t GetInt() const;
    float GetFloat() const;
    glm::vec2 GetVector2() const;
    glm::vec3 GetVector3() const;
    glm::vec4 GetVector4() const;

    std::shared_ptr<ITexture> GetTexture() const;

    void SetInt(int32_t value);
    void SetFloat(float value);
    void SetVector2(glm::vec2 value);
    void SetVector3(glm::vec3 value);
    void SetVector4(glm::vec4 value);
    void SetTexture(const std::shared_ptr<ITexture>& value);

    uint32_t TextureUnit{0};

private:
    ParamVal m_ParamValue;
    std::shared_ptr<ITexture> m_Texture;
    MaterialParamType m_ParamType;
    char m_UniformName[64];
};