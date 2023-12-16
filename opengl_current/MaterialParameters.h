#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>
#include "Texture.h"
#include "Shader.h"

#include <cstring>

template<typename>
struct PropertySetter;

template <typename PropertyType>
struct Property
{
    PropertyType Value;
    char UniformName[96];

    Property() = default;
    Property(const PropertyType& value, const std::string& uniformName):
        Value{value}
    {
        strncpy(UniformName, uniformName.c_str(), 96);
        UniformName[95] = 0;
    }

    Property(const Property<PropertyType>&) = default;
    Property& operator=(const Property<PropertyType>&) = default;
};

template <typename T>
class PropertyList
{
public:
    PropertyList() = default;
    PropertyList(const PropertyList<T>&) = default;
    PropertyList& operator=(const PropertyList<T>&) = default;

public:
    void Add(const T& property, const std::string& uniformName, const std::string& name)
    {
        _properties[name] = Property<T>{ property, uniformName };
    }

    void RefreshVars(Shader& shader) const
    {
        for (auto &[name, property] : _properties)
        {
            PropertySetter<T>::SetValue(shader, property);
        }
    }

    bool GetValue(const char* name, T& outValue) const
    {
        auto it = _properties.find(name);

        if (it != _properties.end())
        {
            outValue = it->second.Value;
            return true;
        }

        return false;
    }

    void SetValue(const char* name, const T& value)
    {
        auto it = _properties.find(name);

        if (it != _properties.end())
        {
            it->second.Value = value;
        }
    }

private:
    std::unordered_map<std::string, Property<T>> _properties;
};


template<>
struct PropertySetter<std::int32_t>
{
    static void SetValue(Shader& shader, const Property<std::int32_t> &property)
    {
        shader.SetUniformInt(property.UniformName, property.Value);
    }
};

template<>
struct PropertySetter<float>
{
    static void SetValue(Shader& shader, const Property<float>& property)
    {
        shader.SetUniformFloat(property.UniformName, property.Value);
    }
};


template<>
struct PropertySetter<glm::vec2>
{
    static void SetValue(Shader& shader, const Property<glm::vec2>& property)
    {
        shader.SetUniformVec2(property.UniformName, property.Value);
    }
};

template<>
struct PropertySetter<glm::vec3>
{
    static void SetValue(Shader& shader, const Property<glm::vec3>& property)
    {
        shader.SetUniformVec3(property.UniformName, property.Value);
    }
};


template<>
struct PropertySetter<glm::vec4>
{
    static void SetValue(Shader& shader, const Property<glm::vec4>& property)
    {
        shader.SetUniformVec4(property.UniformName, property.Value);
    }
};


template<>
struct PropertySetter<std::shared_ptr<Texture>>
{
    static inline std::uint32_t TextureUnit = 0;

    static void SetValue(Shader& shader, const Property<std::shared_ptr<Texture>>& property)
    {
        shader.SetSamplerUniform(property.UniformName, *property.Value, TextureUnit++);
    }
};