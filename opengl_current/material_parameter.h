#pragma once

#include "texture.h"
#include "shader.h"

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include <cstring>
#include <array>

#include <variant>
#include <any>

enum class MaterialParamType : std::int8_t
{
    Unknown = 0,
    Int,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Sampler2D,
    NumMaterialParamsType
};

class Parameter
{
public:
    virtual ~Parameter() = default;

    virtual void SetValue(std::int32_t value)
    {
    }

    virtual void SetValue(float value)
    {
    }

    virtual void SetValue(glm::vec2 value)
    {
    }

    virtual void SetValue(glm::vec3 value)
    {
    }

    virtual void SetValue(glm::vec4 value)
    {
    }

    virtual void SetValue(std::shared_ptr<Texture> texture)
    {
    }

    virtual std::any GetValue() const
    {
        return nullptr;
    }

    virtual void SetUniform(Shader& shader) const = 0;
};

template <typename T>
class PrimitiveParameter : public Parameter
{
public:
    PrimitiveParameter(std::string uniformName, T value) :
        m_UniformName{uniformName},
        m_Value{value}
    {
    }

    PrimitiveParameter(const PrimitiveParameter<T>&) = default;
    PrimitiveParameter& operator=(const PrimitiveParameter<T>&) = default;

    void SetValue(T value) override
    {
        m_Value = value;
    }

    void SetUniform(Shader& shader) const override
    {
        shader.SetUniform(m_UniformName.c_str(), m_Value);
    }

    std::any GetValue() const override
    {
        return m_Value;
    }

private:
    std::string m_UniformName;
    T m_Value;
};

class TextureParameter : public Parameter
{
public:
    TextureParameter(std::string uniformName, std::shared_ptr<Texture> value, std::uint32_t textureUnit) :
        m_UniformName{uniformName},
        m_Texture{value},
        m_TextureUnit{textureUnit}
    {
    }

    TextureParameter(const TextureParameter&) = default;
    TextureParameter& operator=(const TextureParameter&) = default;

    void SetUniform(Shader& shader) const override;
    std::any GetValue() const
    {
        return m_Texture;
    }

private:
    std::string m_UniformName;
    std::shared_ptr<Texture> m_Texture;
    std::uint32_t m_TextureUnit;
};

class UnknownParameter : public Parameter
{
    // Inherited via Parameter
    void SetUniform(Shader& shader) const override;
};

using ParamVariant = std::variant<TextureParameter,
    PrimitiveParameter<std::int32_t>,
    PrimitiveParameter<float>,
    PrimitiveParameter<glm::vec2>,
    PrimitiveParameter<glm::vec3>,
    PrimitiveParameter<glm::vec4>,
    UnknownParameter
>;

class MaterialParam
{
    friend class Material;

public:
    MaterialParam() = default;
    MaterialParam(const MaterialParam&) = default;
    MaterialParam(const char* uniformName, std::int32_t value);
    MaterialParam(const char* uniformName, float value);
    MaterialParam(const char* uniformName, glm::vec2 value);
    MaterialParam(const char* uniformName, glm::vec3 value);
    MaterialParam(const char* uniformName, glm::vec4 value);
    MaterialParam(const char* uniformName, std::shared_ptr<Texture> value, std::uint32_t textureUnit);

    MaterialParam& operator=(const MaterialParam&) = default;
    void SetUniform(Shader& shader) const;

    std::int32_t GetInt() const;
    float GetFloat() const;
    glm::vec2 GetVector2() const;
    glm::vec3 GetVector3() const;
    glm::vec4 GetVector4() const;

    std::shared_ptr<Texture> GetTexture() const;

    void SetInt(std::int32_t value);
    void SetFloat(float value);
    void SetVector2(glm::vec2 value);
    void SetVector3(glm::vec3 value);
    void SetVector4(glm::vec4 value);
    void SetTexture(const std::shared_ptr<Texture>& value);

private:
    ParamVariant m_Parameter{UnknownParameter{}};
    Parameter* (*m_Getter)(const ParamVariant& value);
    MaterialParamType m_ParamType{MaterialParamType::Unknown};
};