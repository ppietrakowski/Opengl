#pragma once

#include "Core.hpp"

#include <cstdint>
#include <string>
#include <optional>

#include <unordered_map>
#include <glm/glm.hpp>
#include <stdexcept>
#include <memory>
#include <span>

#include <array>

#include "UniformBuffer.hpp"

enum class UniformType : uint8_t
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
    int32_t Location;
    int32_t ArraySize{1};
};

struct ShaderCompilationFailedException : public std::runtime_error
{
    ShaderCompilationFailedException(const char* errorMessage);
};

struct ShaderProgramLinkingFailedException : public std::runtime_error
{
    ShaderProgramLinkingFailedException(const char* errorMessage);
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

class Texture;

static constexpr int32_t MinTextureUnits = 32;

class Shader
{
public:
    Shader(std::span<const std::string> sources);
    ~Shader();

public:
    void Use() const;
    void StopUsing() const;

    void SetUniform(const char* name, int32_t value);
    void SetUniform(const char* name, float value);
    void SetUniform(const char* name, glm::vec2 value);
    void SetUniform(const char* name, const glm::vec3& value);
    void SetUniform(const char* name, const glm::vec4& value);

    void SetUniform(const char* name, const glm::mat4& value);
    void SetUniformMat4Array(const char* name, std::span<const glm::mat4> values);
    void SetUniform(const char* name, const glm::mat3& value);

    std::vector<UniformInfo> GetUniformsInfo() const;
    void SetSamplerUniform(const char* uniformName, const std::shared_ptr<Texture>& textures, uint32_t startTextureUnit = 0);
    void SetSamplersUniform(const char* uniformName, std::span<const std::shared_ptr<Texture>> textures, uint32_t startTextureUnit = 0);

    void BindUniformBuffer(int32_t blockIndex, const UniformBuffer& buffer);
    int32_t GetUniformBlockIndex(const std::string& name) const;

    uint32_t GetOpenGlIdentifier() const
    {
        return m_ShaderProgram;
    }

private:
    uint32_t m_ShaderProgram{0};
    mutable std::unordered_map<std::string, int32_t> m_UniformNameToLocation;

private:
    Shader() = default;

private:
    void GenerateShaders(std::span<std::string_view> sources);
    void GenerateShaders(std::span<const std::string> sources);

    int32_t GetUniformLocation(const char* uniformName) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& outUniformsInfo, int32_t location) const;
};

inline ShaderCompilationFailedException::ShaderCompilationFailedException(const char* errorMessage) :
    std::runtime_error{errorMessage}
{
}

inline ShaderProgramLinkingFailedException::ShaderProgramLinkingFailedException(const char* errorMessage) :
    std::runtime_error{errorMessage}
{
}