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
    UniformType Type;
    std::string Name;
    int32_t Location;
    uint32_t NumTextures{0};
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

class ITexture;

static constexpr uint32_t kMinTextureUnits = 16;


class IShader
{
public:
    enum ShaderIndex
    {
        kVertex,
        kFragment,
        kGeometry,
        kTesselationControlShader,
        kTesselationEvaluateShader,
        kCount
    };

public:
    static std::shared_ptr<IShader> CreateFromSource(std::string_view vertexShaderSource, std::string_view fragmentShaderSource);
    static std::shared_ptr<IShader> CreateFromSource(std::string_view vertexShaderSource, std::string_view fragmentShaderSource,
        std::string_view geometryShaderSource);

    static std::shared_ptr<IShader> CreateFromSource(std::string_view vertexShaderSource, std::string_view fragmentShaderSource,
        std::string_view geometryShaderSource, std::string_view tesselationControlShader, std::string_view tesselationEvaluateShader);

    static std::shared_ptr<IShader> LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
    static std::shared_ptr<IShader> LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath,
        std::string_view geometryShaderPath);

    static std::shared_ptr<IShader> LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath,
        std::string_view geometryShaderPath, std::string_view tesselationControlShaderPath, std::string_view tesselationEvaluateShaderPath);

    virtual ~IShader() = default;

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
    virtual void SetSamplerUniform(const char* uniform_name, const std::shared_ptr<ITexture>& textures, uint32_t startTextureUnit = 0) = 0;

protected:
    virtual void GenerateShaders(std::span<std::string_view> sources) = 0;

private:
    static std::shared_ptr<IShader> LoadShader(const std::initializer_list<std::string_view>& paths);
};