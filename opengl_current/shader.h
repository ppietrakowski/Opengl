#pragma once

#include "Core.h"

#include <cstdint>
#include <string>
#include <optional>

#include <unordered_map>
#include <glm/glm.hpp>
#include <stdexcept>
#include <memory>
#include <span>

#include <array>

#include "uniform_buffer.h"

enum class UniformType
{
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

struct UniformInfo
{
    UniformType uniform_type;
    std::string name;
    std::int32_t location;
    int32_t array_size{1};
};

struct ShaderCompilationFailedException : public std::runtime_error
{
    ShaderCompilationFailedException(const char* error_message) :
        std::runtime_error{error_message}
    {
    }
};

struct ShaderProgramLinkingFailedException : public std::runtime_error
{
    ShaderProgramLinkingFailedException(const char* error_message) :
        std::runtime_error{error_message}
    {
    }
};

struct ShaderIndex
{
    enum IndexType
    {
        kVertex = 0,
        kFragment,
        kGeometry,
        kTesselationControlShader,
        kTesselationEvaluateShader,
        kCount
    };
};

class Shader;

class ShaderSourceBuilder
{
public:
    std::shared_ptr<Shader> Build();

    ShaderSourceBuilder& SetVertexShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadVertexShaderSource(const std::filesystem::path& file_path);

    ShaderSourceBuilder& SetFragmentShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadFragmentShaderSource(const std::filesystem::path& file_path);

    ShaderSourceBuilder& SetGeometryShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadGeometryShaderSource(const std::filesystem::path& file_path);

    ShaderSourceBuilder& SetTesselationShaderSource(const std::string& control_shader_source, const std::string& evaluate_shader_source);
    ShaderSourceBuilder& LoadGeometryShaderSource(const std::filesystem::path& control_shader_path, const std::filesystem::path& evaluate_shader_path);

private:
    std::array<std::string, ShaderIndex::kCount> shader_sources_;

    std::uint32_t GetLastShaderIndex() const;
};


class Texture;

static constexpr std::int32_t kMinTextureUnits = 16;

class Shader
{
public:
    Shader(std::span<const std::string> sources);
    ~Shader();

public:
    void Use() const;
    void StopUsing() const;

    void SetUniform(const char* name, std::int32_t value);
    void SetUniform(const char* name, float value);
    void SetUniform(const char* name, glm::vec2 value);
    void SetUniform(const char* name, const glm::vec3& value);
    void SetUniform(const char* name, const glm::vec4& value);

    void SetUniform(const char* name, const glm::mat4& value);
    void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values);
    void SetUniform(const char* name, const glm::mat3& value);

    int GetUniformInt(const char* name) const;

    float GetUniformFloat(const char* name) const;
    glm::vec2 GetUniformVec2(const char* name) const;
    glm::vec3 GetUniformVec3(const char* name) const;
    glm::vec4 GetUniformVec4(const char* name) const;
                                                    ;
    std::vector<UniformInfo> GetUniformsInfo() const;
    void SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, std::uint32_t start_texture_unit = 0);

    void BindUniformBuffer(std::int32_t block_index, const UniformBuffer& buffer);
    int GetUniformBlockIndex(const std::string& name) const;

private:
    std::uint32_t shader_program_{0};
    mutable std::unordered_map<std::string, int> uniform_name_to_location_;

private:
    Shader() = default;

    void GenerateShaders(std::span<std::string_view> sources);
    void GenerateShaders(std::span<const std::string> sources);

    int GetUniformLocation(const char* uniform_name) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& out_uniforms_info, int location) const;
};