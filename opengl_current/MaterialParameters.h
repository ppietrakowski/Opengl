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
struct TProperty {
    PropertyType value;
    char uniform_name[96];

    TProperty() = default;
    TProperty(const PropertyType& value, const std::string& uniform_name) :
        value{ value } {
        strncpy(this->uniform_name, uniform_name.c_str(), 96);
        this->uniform_name[95] = 0;
    }

    TProperty(const TProperty<PropertyType>&) = default;
    TProperty& operator=(const TProperty<PropertyType>&) = default;
};

template <typename T>
class MaterialPropertyList {
public:
    MaterialPropertyList() = default;
    MaterialPropertyList(const MaterialPropertyList<T>&) = default;
    MaterialPropertyList& operator=(const MaterialPropertyList<T>&) = default;

public:
    void Add(const T& property, const std::string& uniform_name, const std::string& name);

    /// <summary>
    /// Sets all material uniforms to passed shader
    /// </summary>
    /// <param name="shader"> Shader to update </param>
    void RefreshVars(Shader& shader) const;

    /// <summary>
    /// Retrieves value of this material uniform
    /// </summary>
    /// <param name="shader"> Shader to update </param>
    /// <exception cref="std::runtime_error">When value with this name doesn't exists</exception>
    T GetValue(const char* name) const;

    void SetValue(const char* name, const T& value);

private:
    std::unordered_map<std::string, TProperty<T>> properties_;
};

template <typename T>
inline void MaterialPropertyList<T>::Add(const T& property, const std::string& uniform_name, const std::string& name) {
    properties_[name] = TProperty<T>{ property, uniform_name };
}

template <typename T>
inline void MaterialPropertyList<T>::RefreshVars(Shader& shader) const {
    for (auto& [name, property] : properties_) {
        PropertySetter<T>::SetValue(shader, property);
    }
}

template <typename T>
inline T MaterialPropertyList<T>::GetValue(const char* name) const {
    auto it = properties_.find(name);

    if (it != properties_.end()) {
        return it->second.value;
    }

    throw std::runtime_error{ std::string{"Variable "} + name + " doesn't exists in this property list" };
}

template <typename T>
inline void MaterialPropertyList<T>::SetValue(const char* name, const T& value) {
    auto it = properties_.find(name);

    if (it != properties_.end()) {
        it->second.value = value;
    }
}

template<>
struct PropertySetter<std::int32_t> {
    static void SetValue(Shader& shader, const TProperty<std::int32_t>& property) {
        shader.SetUniformInt(property.uniform_name, property.value);
    }
};

template<>
struct PropertySetter<float> {
    static void SetValue(Shader& shader, const TProperty<float>& property) {
        shader.SetUniformFloat(property.uniform_name, property.value);
    }
};


template<>
struct PropertySetter<glm::vec2> {
    static void SetValue(Shader& shader, const TProperty<glm::vec2>& property) {
        shader.SetUniformVec2(property.uniform_name, property.value);
    }
};

template<>
struct PropertySetter<glm::vec3> {
    static void SetValue(Shader& shader, const TProperty<glm::vec3>& property) {
        shader.SetUniformVec3(property.uniform_name, property.value);
    }
};


template<>
struct PropertySetter<glm::vec4> {
    static void SetValue(Shader& shader, const TProperty<glm::vec4>& property) {
        shader.SetUniformVec4(property.uniform_name, property.value);
    }
};


template<>
struct PropertySetter<std::shared_ptr<Texture>> {
    static inline std::uint32_t texture_unit = 0;

    static void SetValue(Shader& shader, const TProperty<std::shared_ptr<Texture>>& property) {
        shader.SetSamplerUniform(property.uniform_name, *property.value, texture_unit++);
    }
};