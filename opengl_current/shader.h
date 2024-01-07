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
    UniformType uniform_type;
    std::string name;
    std::int32_t location;
    int32_t num_textures{0};
};

struct ShaderCompilationFailedException : public std::runtime_error {
    ShaderCompilationFailedException(const char* error_message) :
        std::runtime_error{error_message}
    {
    }
};

struct ShaderProgramLinkingFailedException : public std::runtime_error {
    ShaderProgramLinkingFailedException(const char* error_message) :
        std::runtime_error{error_message}
    {
    }
};

struct ShaderIndex {
    enum IndexType {
        kVertex = 0,
        kFragment,
        kGeometry,
        kTesselationControlShader,
        kTesselationEvaluateShader,
        kCount
    };
};

class Shader;

class ShaderSourceBuilder {
public:
    std::shared_ptr<Shader> Build();

    ShaderSourceBuilder& SetVertexShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadVertexShaderSource(const std::filesystem::path& file_path);

    ShaderSourceBuilder& SetFragmentShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadFragmentShaderSource(const std::filesystem::path& file_path);

    ShaderSourceBuilder& SetGeometryShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadGeometryShaderSource(const std::filesystem::path& file_path);

    ShaderSourceBuilder& SetTesselationShaderSource(const std::string& control_shader_source, const std::string& evaluate_shader_source);
    ShaderSourceBuilder& LoadGeometryShaderSource(const std::filesystem::path& control_shader_path, const std::filesystem::path& evaluate_shader_source);

private:
    std::array<std::string, ShaderIndex::kCount> shader_sources_;

    std::uint32_t GetLastShaderIndex() const;
};


class Texture;

static constexpr std::int32_t kMinTextureUnits = 16;

class Shader {
    friend class Shader;
    friend class ShaderSourceBuilder;
public:
    static std::shared_ptr<Shader> CreateFromSource(std::span<const std::string> sources);
    virtual ~Shader() = default;

public:
    virtual void Use() const = 0;
    virtual void StopUsing() const = 0;

    virtual void SetUniformInt(const char* name, int value) = 0;
    virtual void SetUniformFloat(const char* name, float value) = 0;
    virtual void SetUniformVec2(const char* name, glm::vec2 value) = 0;
    virtual void SetUniformVec3(const char* name, const glm::vec3& value) = 0;
    virtual void SetUniformVec4(const char* name, const glm::vec4& value) = 0;

    virtual void SetUniformMat4(const char* name, const glm::mat4& value) = 0;
    virtual void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, std::uint32_t count) = 0;
    virtual void SetUniformMat3(const char* name, const glm::mat3& value) = 0;

    virtual int GetUniformInt(const char* name) const = 0;

    virtual float GetUniformFloat(const char* name) const = 0;
    virtual glm::vec2 GetUniformVec2(const char* name) const = 0;
    virtual glm::vec3 GetUniformVec3(const char* name) const = 0;
    virtual glm::vec4 GetUniformVec4(const char* name) const = 0;

    virtual std::vector<UniformInfo> GetUniformInfos() const = 0;
    virtual void SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, std::uint32_t start_texture_unit = 0) = 0;

protected:
    virtual void GenerateShaders(std::span<std::string_view> sources) = 0;
    void GenerateShaders(std::span<const std::string> sources);
};