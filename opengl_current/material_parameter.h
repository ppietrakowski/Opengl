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

enum class MaterialParamType : int8_t
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

class Parameter;
class TextureParameter;

template<typename>
class PrimitiveParameter;

class IMaterialParameterVisitor
{
public:
    virtual ~IMaterialParameterVisitor() = default;

    virtual void Visit(const Parameter& defaultParam, const std::string& name)
    {
    }

    virtual void Visit(PrimitiveParameter<int32_t>& param, const std::string& name) = 0;
    virtual void Visit(PrimitiveParameter<float>& param, const std::string& name) = 0;
    virtual void Visit(PrimitiveParameter<glm::vec2>& param, const std::string& name) = 0;
    virtual void Visit(PrimitiveParameter<glm::vec3>& param, const std::string& name) = 0;
    virtual void Visit(PrimitiveParameter<glm::vec4>& param, const std::string& name) = 0;
    virtual void Visit(TextureParameter& param, const std::string& name) = 0;
};

class Parameter
{
public:
    virtual ~Parameter() = default;

    virtual void SetValue(int32_t value)
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

    virtual void Accept(IMaterialParameterVisitor& visitor, const std::string& name)
    {
        visitor.Visit(*this, name);
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

    T GetPrimitiveValue() const
    {
        return m_Value;
    }

    void Accept(IMaterialParameterVisitor& visitor, const std::string& name) override
    {
        visitor.Visit(*this, name);
    }

private:
    std::string m_UniformName;
    T m_Value;
};

class TextureParameter : public Parameter
{
public:
    TextureParameter(std::string uniformName, std::shared_ptr<Texture> value, uint32_t textureUnit) :
        m_UniformName{uniformName},
        m_Texture{value},
        m_TextureUnit{textureUnit}
    {
    }

    void SetValue(std::shared_ptr<Texture> texture) override
    {
        m_Texture = texture;
    }

    TextureParameter(const TextureParameter&) = default;
    TextureParameter& operator=(const TextureParameter&) = default;

    void SetUniform(Shader& shader) const override;
    std::any GetValue() const
    {
        return m_Texture;
    }

    std::shared_ptr<Texture> GetPrimitiveValue() const
    {
        return m_Texture;
    }
    
    uint32_t GetTextureUnitAssigned() const
    {
        return m_TextureUnit;
    }

    void Accept(IMaterialParameterVisitor& visitor, const std::string& name) override
    {
        visitor.Visit(*this, name);
    }


private:
    std::string m_UniformName;
    std::shared_ptr<Texture> m_Texture;
    uint32_t m_TextureUnit;
};

class UnknownParameter : public Parameter
{
    // Inherited via Parameter
    void SetUniform(Shader& shader) const override;
};

using ParamVariant = std::variant<TextureParameter,
    PrimitiveParameter<int32_t>,
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
    MaterialParam(const char* uniformName, int32_t value);
    MaterialParam(const char* uniformName, float value);
    MaterialParam(const char* uniformName, glm::vec2 value);
    MaterialParam(const char* uniformName, glm::vec3 value);
    MaterialParam(const char* uniformName, glm::vec4 value);
    MaterialParam(const char* uniformName, std::shared_ptr<Texture> value, uint32_t textureUnit);

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

    void Accept(IMaterialParameterVisitor& visitor, const std::string& name);

private:
    ParamVariant m_Parameter{UnknownParameter{}};
    Parameter* (*m_Getter)(const ParamVariant& value);
    MaterialParamType m_ParamType{MaterialParamType::Unknown};
};