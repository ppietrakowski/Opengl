#pragma once

#include "MaterialParameters.h"

class Material
{
public:
    Material(const std::shared_ptr<Shader>& shader);

    Material(const Material&) = default;
    Material& operator=(const Material&) = default;

public:
    bool GetIntProperty(const char* name, std::int32_t& outInt) const;
    void SetIntProperty(const char* name, std::int32_t value);

    bool GetFloatProperty(const char* name, float& outFloat) const;
    void SetFloatProperty(const char* name, float value);

    bool GetVector2Property(const char* name, glm::vec2& outVec) const;
    void SetVector2Property(const char* name, glm::vec2 value);

    bool GetVector3Property(const char* name, glm::vec3& outVec) const;
    void SetVector3Property(const char* name, glm::vec3 value);

    bool GetVector4Property(const char* name, glm::vec4& outVec) const;
    void SetVector4Property(const char* name, glm::vec4 value);

    bool GetTextureProperty(const char* name, std::shared_ptr<Texture>& outTexture) const;
    void SetTextureProperty(const char* name, const std::shared_ptr<Texture>& value);

    void SetupRenderState() const;
    void SetShaderUniforms() const;

    Shader& GetShader() const { return *_shader; }

public:

    bool UseWireframe : 1{ false };
    bool ShouldCullFaces : 1{ true };
    bool UsingTransparency : 1{ false };

private:
    PropertyList<std::int32_t> _ints;
    PropertyList<float> _floats;
    PropertyList<glm::vec2> _vectors2;
    PropertyList<glm::vec3> _vectors3;
    PropertyList<glm::vec4> _vectors4;
    PropertyList<std::shared_ptr<Texture>> _textures;
    std::shared_ptr<Shader> _shader;


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
