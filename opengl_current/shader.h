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

enum class UniformType
{
    Undefined,
    Vec4,
    Vec3,
    Vec2,
    Float,
    Int,
    Ivec2,
    Ivec3,
    Mat4x4,
    Mat3x3,
    Boolean,
    Sampler2D,
};

struct UniformInfo
{
    UniformType Type;
    std::string Name;
    std::int32_t Location;
    int32_t NumTextures{0};
};

struct ShaderCompilationFailedException : public std::runtime_error
{
    ShaderCompilationFailedException(const char* errorMessage) :
        std::runtime_error{errorMessage}
    {
    }
};

struct ShaderProgramLinkingFailedException : public std::runtime_error
{
    ShaderProgramLinkingFailedException(const char* errorMessage) :
        std::runtime_error{errorMessage}
    {
    }
};

struct ShaderIndex
{
    enum IndexType
    {
        Vertex = 0,
        Fragment,
        Geometry,
        TesselationControlShader,
        TesselationEvaluateShader,
        Count
    };
};

class Shader;

class ShaderSourceBuilder
{
public:
    std::shared_ptr<Shader> Build();

    ShaderSourceBuilder& SetVertexShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadVertexShaderSource(const std::filesystem::path& filePath);

    ShaderSourceBuilder& SetFragmentShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadFragmentShaderSource(const std::filesystem::path& filePath);

    ShaderSourceBuilder& SetGeometryShaderSource(const std::string& source);
    ShaderSourceBuilder& LoadGeometryShaderSource(const std::filesystem::path& filePath);

    ShaderSourceBuilder& SetTesselationShaderSource(const std::string& controlShaderSource, const std::string& evaluateShaderSource);
    ShaderSourceBuilder& LoadGeometryShaderSource(const std::filesystem::path& controlShaderPath, const std::filesystem::path& evaluateShaderPath);

private:
    std::array<std::string, ShaderIndex::Count> m_ShaderSources;

    std::uint32_t GetLastShaderIndex() const;
};


class Texture;

static constexpr std::int32_t MinTextureUnits = 16;

class Shader
{
    friend class Shader;
    friend class ShaderSourceBuilder;
public:
    static std::shared_ptr<Shader> CreateFromSource(std::span<const std::string> sources);
    virtual ~Shader() = default;

public:
    virtual void Use() const = 0;
    virtual void StopUsing() const = 0;

    virtual void SetUniform(const char* name, std::int32_t value) = 0;
    virtual void SetUniform(const char* name, float value) = 0;
    virtual void SetUniform(const char* name, glm::vec2 value) = 0;
    virtual void SetUniform(const char* name, const glm::vec3& value) = 0;
    virtual void SetUniform(const char* name, const glm::vec4& value) = 0;

    virtual void SetUniform(const char* name, const glm::mat4& value) = 0;
    virtual void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, std::uint32_t count) = 0;
    virtual void SetUniform(const char* name, const glm::mat3& value) = 0;

    virtual int GetUniformInt(const char* name) const = 0;

    virtual float GetUniformFloat(const char* name) const = 0;
    virtual glm::vec2 GetUniformVec2(const char* name) const = 0;
    virtual glm::vec3 GetUniformVec3(const char* name) const = 0;
    virtual glm::vec4 GetUniformVec4(const char* name) const = 0;

    virtual std::vector<UniformInfo> GetUniformInfos() const = 0;
    virtual void SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, std::uint32_t startTextureUnit = 0) = 0;

protected:
    virtual void GenerateShaders(std::span<std::string_view> sources) = 0;
    void GenerateShaders(std::span<const std::string> sources);
};