#pragma once

#include "material_parameter.h"

class Material {
public:
    Material(const std::shared_ptr<Shader>& shader);

    Material(const Material&) = default;
    Material& operator=(const Material&) = default;

public:
    int GetIntProperty(const char* name) const;
    void SetIntProperty(const char* name, int value);

    float GetFloatProperty(const char* name) const;
    void SetFloatProperty(const char* name, float value);

    glm::vec2 GetVector2Property(const char* name) const;
    void SetVector2Property(const char* name, glm::vec2 value);

    glm::vec3 GetVector3Property(const char* name) const;
    void SetVector3Property(const char* name, glm::vec3 value);

    glm::vec4 GetVector4Property(const char* name) const;
    void SetVector4Property(const char* name, glm::vec4 value);

    std::shared_ptr<Texture> GetTextureProperty(const char* name) const;
    void SetTextureProperty(const char* name, const std::shared_ptr<Texture>& value);

    void SetupRenderState() const;
    void SetShaderUniforms() const;

    Shader& GetShader() const {
        return *shader_;
    }

    uint32_t GetNumTextures() const {
        return num_texture_units_;
    }

public:

    bool use_wireframe : 1{ false };
    bool cull_faces : 1{ true };
    bool transparent : 1{ false };

private:
    std::shared_ptr<Shader> shader_;
    std::unordered_map<std::string, MaterialParam> material_params_;
    uint32_t num_texture_units_{0};

private:
    void TryAddNewProperty(const UniformInfo& info);
    void AddNewProperty(const UniformInfo& info);

    MaterialParam& GetParam(const char* name) {
        return material_params_.at(name);
    }

    const MaterialParam& GetParam(const char* name) const {
        return material_params_.at(name);
    }
};

