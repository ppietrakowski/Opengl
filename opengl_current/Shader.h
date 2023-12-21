#pragma once

#include "Core.h"

#include <cstdint>
#include <GL/glew.h>
#include <string>
#include <optional>

#include <unordered_map>
#include <glm/glm.hpp>
#include <stdexcept>
#include <memory>
#include <span>

enum class UniformType {
    kUndefined,
    kVec4,
    kVec3,
    kVec2,
    kFloat,
    kInt,
    kIvec2,
    kIvec3,
    kMat4x4,
    kMat3x3,
    kBoolean,
    kSampler2D,
};

struct UniformInfo {
    UniformType vertex_type;
    std::string name;
    std::int32_t location;

    std::uint32_t num_textures{ 0 };
};

struct ShaderCompilationFailedException : public std::runtime_error {
    ShaderCompilationFailedException(const char* error_message) :
        std::runtime_error{ error_message }
    {
    }
};

struct ShaderProgramLinkingFailedException : public std::runtime_error {
    ShaderProgramLinkingFailedException(const char* error_message) :
        std::runtime_error{ error_message }
    {
    }
};

class Texture;

static constexpr std::uint32_t kMinTextureUnits = 16;


class Shader {
    enum ShaderIndex {
        kVertex,
        kFragment,
        kGeometry,
        kTesselationControlShader,
        kTesselationEvaluateShader,
        kCount
    };

public:
    Shader() = default;
    Shader(std::string_view vertex_shader_source, std::string_view fragment_shader_source);
    Shader(std::string_view vertex_shader_source, std::string_view fragment_shader_source, std::string_view geometry_shader_source);
    Shader(std::string_view vertex_shader_source, std::string_view fragment_shader_source, std::string_view geometry_shader_source,
        std::string_view tesselation_control_shader, std::string_view tesselation_evaluate_shader);

    static std::shared_ptr<Shader> LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path);
    static std::shared_ptr<Shader> LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path,
        std::string_view geometry_shader_path);

    static std::shared_ptr<Shader> LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path,
        std::string_view geometry_shader_path, std::string_view tesselationControlShaderPath, std::string_view tesselationEvaluateShaderPath);

    Shader(Shader&& shader) noexcept;
    Shader& operator=(Shader&& shader) noexcept;

    ~Shader();

public:
    void Use() const;
    void StopUsing() const;

    void SetUniformInt(const char* name, std::int32_t value);
    void SetUniformFloat(const char* name, float value);
    void SetUniformVec2(const char* name, glm::vec2 value);
    void SetUniformVec3(const char* name, const glm::vec3& value);
    void SetUniformVec4(const char* name, const glm::vec4& value);

    void SetUniformMat4(const char* name, const glm::mat4& value);
    void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, std::uint32_t count);
    void SetUniformMat3(const char* name, const glm::mat3& value);

    std::int32_t GetUniformInt(const char* name) const;

    float GetUniformFloat(const char* name) const;
    glm::vec2 GetUniformVec2(const char* name) const;
    glm::vec3 GetUniformVec3(const char* name) const;
    glm::vec4 GetUniformVec4(const char* name) const;

    std::vector<UniformInfo> GetUniformInfos() const;
    void SetSamplerUniform(const char* uniform_name, std::span<const std::shared_ptr<Texture>> textures, std::uint32_t count, std::uint32_t start_texture_unit=0);

private:
    GLuint shader_program_{ 0 };
    mutable std::unordered_map<std::string, std::int32_t> uniform_locations_cache_;

private:
    static std::shared_ptr<Shader> LoadShader(const std::initializer_list<std::string_view>& paths);
    void GenerateShaders(std::span<std::string_view> sources);

private:
    std::int32_t GetUniformLocation(const char* uniform_name) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& out_uniforms_info, GLint location) const;
};