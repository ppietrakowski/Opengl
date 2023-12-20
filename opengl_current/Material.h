#pragma once

#include "MaterialParameters.h"

class Material
{
public:
    Material(const std::shared_ptr<Shader>& shader);

    Material(const Material&) = default;
    Material& operator=(const Material&) = default;

public:
    std::int32_t GetIntProperty(const char* name) const;
    void SetIntProperty(const char* name, std::int32_t value);

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

    Shader& GetShader() const { return *shader_; }

public:

    bool use_wireframe : 1{ false };
    bool should_cull_faces : 1{ true };
    bool using_transparency : 1{ false };

private:
    MaterialPropertyList<std::int32_t> ints_;
    MaterialPropertyList<float> floats_;
    MaterialPropertyList<glm::vec2> vectors2_;
    MaterialPropertyList<glm::vec3> vectors3_;
    MaterialPropertyList<glm::vec4> vectors4_;
    MaterialPropertyList<std::shared_ptr<Texture>> textures_;
    std::shared_ptr<Shader> shader_;


private:
    void TryAddNewProperty(const UniformInfo& info);
    void AddNewProperty(const UniformInfo& info);
    void AddNewTexture(const UniformInfo& info);
    void AddNewInt(const UniformInfo& info);

    void AddNewFloat(const UniformInfo& info);
    void AddNewVec2(const UniformInfo& info);
    void AddNewVec3(const UniformInfo& info);
    void AddNewVec4(const UniformInfo& info);
};
