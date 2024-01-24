#pragma once

#include "material_parameter.h"

class Material
{
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

    Shader& GetShader() const
    {
        return *m_Shader;
    }

    uint32_t GetNumTextures() const
    {
        return m_NumTextureUnits;
    }

public:

    bool bUseWireframe : 1{ false };
    bool bCullFaces : 1{ true };
    bool bTransparent : 1{ false };

private:
    std::shared_ptr<Shader> m_Shader;
    std::unordered_map<std::string, MaterialParam> m_MaterialParams;
    uint32_t m_NumTextureUnits{0};

private:
    void TryAddNewProperty(const UniformInfo& info);
    void AddNewProperty(const UniformInfo& info);

    MaterialParam& GetParam(const char* name)
    {
        return m_MaterialParams.at(name);
    }

    const MaterialParam& GetParam(const char* name) const
    {
        return m_MaterialParams.at(name);
    }
};

