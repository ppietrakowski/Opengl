#pragma once

#include <GL/glew.h>
#include "shader.h"

class OpenGlShader : public Shader {
public:
    OpenGlShader();

    OpenGlShader(std::string_view vertex_shader_source, std::string_view fragment_shader_source);
    OpenGlShader(std::string_view vertex_shader_source, std::string_view fragment_shader_source, std::string_view geometry_shader_source);
    OpenGlShader(std::string_view vertex_shader_source, std::string_view fragment_shader_source, std::string_view geometry_shader_source,
        std::string_view tesselation_control_shader, std::string_view tesselation_evaluate_shader);

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
    virtual void SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, uint32_t start_texture_unit) override;

    void GenerateShaders(std::span<std::string_view> sources) override;

private:
    GLuint shader_program_{ 0 };
    mutable std::unordered_map<std::string, GLint> uniform_locations_cache_;

private:
    
    int32_t GetUniformLocation(const char* uniform_name) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& out_uniforms_info, int32_t location) const;
};

