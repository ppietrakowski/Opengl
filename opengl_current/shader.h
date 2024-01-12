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
    void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, std::uint32_t count);
    void SetUniform(const char* name, const glm::mat3& value);

    int GetUniformInt(const char* name) const;

    float GetUniformFloat(const char* name) const;
    glm::vec2 GetUniformVec2(const char* name) const;
    glm::vec3 GetUniformVec3(const char* name) const;
    glm::vec4 GetUniformVec4(const char* name) const;
                                                    ;
    std::vector<UniformInfo> GetUniformsInfo() const;
    void SetSamplerUniform(const char* uniformName, const std::shared_ptr<Texture>& textures, std::uint32_t startTextureUnit = 0);

private:
    std::uint32_t m_ShaderProgram{0};
    mutable std::unordered_map<std::string, int> m_UniformNameToLocation;

private:
    Shader() = default;

    void GenerateShaders(std::span<std::string_view> sources);
    void GenerateShaders(std::span<const std::string> sources);

    int GetUniformLocation(const char* uniformName) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& outUniformsInfo, int location) const;
};