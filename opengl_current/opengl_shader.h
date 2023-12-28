#pragma once

#include <GL/glew.h>
#include "shader.h"

class OpenGlShader : public IShader
{
public:
    OpenGlShader();

    OpenGlShader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource);
    OpenGlShader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource, std::string_view geometryShaderSource);
    OpenGlShader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource, std::string_view geometryShaderSource,
        std::string_view tesselationControlShader, std::string_view tesselationEvaluateShader);

    ~OpenGlShader();

public:

    virtual void Use() const override;
    virtual void StopUsing() const override;

    virtual void SetUniformInt(const char* name, int32_t value) override;
    virtual void SetUniformFloat(const char* name, float value) override;
    virtual void SetUniformVec2(const char* name, glm::vec2 value) override;
    virtual void SetUniformVec3(const char* name, const glm::vec3& value) override;
    virtual void SetUniformVec4(const char* name, const glm::vec4& value) override;

    virtual void SetUniformMat4(const char* name, const glm::mat4& value) override;
    virtual void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, uint32_t count) override;
    virtual void SetUniformMat3(const char* name, const glm::mat3& value) override;

    virtual int32_t GetUniformInt(const char* name) const override;

    virtual float GetUniformFloat(const char* name) const override;
    virtual glm::vec2 GetUniformVec2(const char* name) const override;
    virtual glm::vec3 GetUniformVec3(const char* name) const override;
    virtual glm::vec4 GetUniformVec4(const char* name) const override;

    virtual std::vector<UniformInfo> GetUniformInfos() const override;
    virtual void SetSamplerUniform(const char* uniformName, const std::shared_ptr<ITexture>& textures, uint32_t startTextureUnit) override;

    void GenerateShaders(std::span<std::string_view> sources) override;

private:
    GLuint m_ShaderProgram{0};
    mutable std::unordered_map<std::string, GLint> m_UniformNameToLocation;

private:

    int32_t GetUniformLocation(const char* uniformName) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& outUniformsInfo, int32_t location) const;
};

