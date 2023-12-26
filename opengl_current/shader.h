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
    int32_t location;

    uint32_t num_textures{ 0 };
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

static constexpr uint32_t kMinTextureUnits = 16;


class Shader {
public:
    enum ShaderIndex {
        kVertex,
        kFragment,
        kGeometry,
        kTesselationControlShader,
        kTesselationEvaluateShader,
        kCount
    };

public:
    static std::shared_ptr<Shader> CreateFromSource(std::string_view vertex_shader_source, std::string_view fragment_shader_source);
    static std::shared_ptr<Shader> CreateFromSource(std::string_view vertex_shader_source, std::string_view fragment_shader_source,
        std::string_view geometry_shader_source);

    static std::shared_ptr<Shader> CreateFromSource(std::string_view vertex_shader_source, std::string_view fragment_shader_source,
        std::string_view geometry_shader_source, std::string_view tesselation_control_shader, std::string_view tesselation_evaluate_shader);

    static std::shared_ptr<Shader> LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path);
    static std::shared_ptr<Shader> LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path,
        std::string_view geometry_shader_path);

    static std::shared_ptr<Shader> LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path,
        std::string_view geometry_shader_path, std::string_view tesselationControlShaderPath, std::string_view tesselationEvaluateShaderPath);

    virtual ~Shader() = default;

public:
    virtual void Use() const = 0;
    virtual void StopUsing() const = 0;

    virtual void SetUniformInt(const char* name, int32_t value) = 0;
    virtual void SetUniformFloat(const char* name, float value) = 0;
    virtual void SetUniformVec2(const char* name, glm::vec2 value) = 0;
    virtual void SetUniformVec3(const char* name, const glm::vec3& value) = 0;
    virtual void SetUniformVec4(const char* name, const glm::vec4& value) = 0;

    virtual void SetUniformMat4(const char* name, const glm::mat4& value) = 0;
    virtual void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, uint32_t count) = 0;
    virtual void SetUniformMat3(const char* name, const glm::mat3& value) = 0;

    virtual int32_t GetUniformInt(const char* name) const = 0;

    virtual float GetUniformFloat(const char* name) const = 0;
    virtual glm::vec2 GetUniformVec2(const char* name) const = 0;
    virtual glm::vec3 GetUniformVec3(const char* name) const = 0;
    virtual glm::vec4 GetUniformVec4(const char* name) const = 0;

    virtual std::vector<UniformInfo> GetUniformInfos() const = 0;
    virtual void SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, uint32_t start_texture_unit = 0) = 0;

protected:
    virtual void GenerateShaders(std::span<std::string_view> sources) = 0;

private:
    static std::shared_ptr<Shader> LoadShader(const std::initializer_list<std::string_view>& paths);
};