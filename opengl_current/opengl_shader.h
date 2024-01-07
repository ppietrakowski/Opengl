#pragma once

#include <GL/glew.h>
#include "shader.h"

class OpenGlShader : public Shader {
public:
    OpenGlShader();
    ~OpenGlShader();

public:

    void Use() const override;
    void StopUsing() const override;

    void SetUniformInt(const char* name, std::int32_t value) override;
    void SetUniformFloat(const char* name, float value) override;
    void SetUniformVec2(const char* name, glm::vec2 value) override;
    void SetUniformVec3(const char* name, const glm::vec3& value) override;
    void SetUniformVec4(const char* name, const glm::vec4& value) override;

    void SetUniformMat4(const char* name, const glm::mat4& value) override;
    void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, std::uint32_t count) override;
    void SetUniformMat3(const char* name, const glm::mat3& value) override;

    std::int32_t GetUniformInt(const char* name) const override;

    float GetUniformFloat(const char* name) const override;
    glm::vec2 GetUniformVec2(const char* name) const override;
    glm::vec3 GetUniformVec3(const char* name) const override;
    glm::vec4 GetUniformVec4(const char* name) const override;

    std::vector<UniformInfo> GetUniformInfos() const override;
    void SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, std::uint32_t start_texture_unit) override;

    void GenerateShaders(std::span<std::string_view> sources) override;

private:
    GLuint shader_program_{0};
    mutable std::unordered_map<std::string, GLint> uniform_name_to_location_;

private:

    GLint GetUniformLocation(const char* uniform_name) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& out_uniforms_info, GLint location) const;
};

