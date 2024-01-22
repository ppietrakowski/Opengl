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

class Parameter {
public:
    virtual ~Parameter() = default;

    virtual void SetValue(int value) {
    }

    virtual void SetValue(float value) {
    }

    virtual void SetValue(glm::vec2 value) {
    }

    virtual void SetValue(glm::vec3 value) {
    }

    virtual void SetValue(glm::vec4 value) {
    }

    virtual void SetValue(std::shared_ptr<Texture> texture) {
    }

    virtual std::any GetValue() const {
        return nullptr;
    }

    virtual void SetUniform(Shader& shader) const = 0;
};

template <typename T>
class PrimitiveParameter : public Parameter {
public:
    PrimitiveParameter(std::string uniform_name, T value) :
        uniform_name_{uniform_name},
        value_{value} {
    }

    PrimitiveParameter(const PrimitiveParameter<T>&) = default;
    PrimitiveParameter& operator=(const PrimitiveParameter<T>&) = default;

    void SetValue(T value) override {
        value_ = value;
    }

    void SetUniform(Shader& shader) const override {
        shader.SetUniform(uniform_name_.c_str(), value_);
    }

    std::any GetValue() const override {
        return value_;
    }

private:
    std::string uniform_name_;
    T value_;
};

class TextureParameter : public Parameter {
public:
    TextureParameter(std::string uniform_name, std::shared_ptr<Texture> value, uint32_t texture_unit) :
        uniform_name_{uniform_name},
        texture_{value},
        texture_unit_{texture_unit} {
    }

    void SetValue(std::shared_ptr<Texture> texture) override {
        texture_ = texture;
    }

    TextureParameter(const TextureParameter&) = default;
    TextureParameter& operator=(const TextureParameter&) = default;

    void SetUniform(Shader& shader) const override;
    std::any GetValue() const {
        return texture_;
    }

private:
    std::string uniform_name_;
    std::shared_ptr<Texture> texture_;
    uint32_t texture_unit_;
};

class UnknownParameter : public Parameter {
    // Inherited via Parameter
    void SetUniform(Shader& shader) const override;
};

using ParamVariant = std::variant<TextureParameter,
    PrimitiveParameter<int>,
    PrimitiveParameter<float>,
    PrimitiveParameter<glm::vec2>,
    PrimitiveParameter<glm::vec3>,
    PrimitiveParameter<glm::vec4>,
    UnknownParameter
>;

class MaterialParam {
    friend class Material;

public:
    MaterialParam() = default;
    MaterialParam(const MaterialParam&) = default;
    MaterialParam(const char* uniform_name, int value);
    MaterialParam(const char* uniform_name, float value);
    MaterialParam(const char* uniform_name, glm::vec2 value);
    MaterialParam(const char* uniform_name, glm::vec3 value);
    MaterialParam(const char* uniform_name, glm::vec4 value);
    MaterialParam(const char* uniform_name, std::shared_ptr<Texture> value, uint32_t textureUnit);

    MaterialParam& operator=(const MaterialParam&) = default;
    void SetUniform(Shader& shader) const;

    int GetInt() const;
    float GetFloat() const;
    glm::vec2 GetVector2() const;
    glm::vec3 GetVector3() const;
    glm::vec4 GetVector4() const;

    std::shared_ptr<Texture> GetTexture() const;

    void SetInt(int value);
    void SetFloat(float value);
    void SetVector2(glm::vec2 value);
    void SetVector3(glm::vec3 value);
    void SetVector4(glm::vec4 value);
    void SetTexture(const std::shared_ptr<Texture>& value);

private:
    ParamVariant parameter_{UnknownParameter{}};
    Parameter* (*getter_)(const ParamVariant& value);
    MaterialParamType param_type_{MaterialParamType::kUnknown};
};