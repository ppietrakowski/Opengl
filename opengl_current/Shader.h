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

enum class EUniformType
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
    EUniformType Type;
    std::string Name;
    int Location;
};

struct ShaderCompilationFailedException : public std::runtime_error
{
    ShaderCompilationFailedException(const char* errorMessage):
        std::runtime_error{ errorMessage }
    {
    }
};

struct ShaderProgramLinkingFailedException : public std::runtime_error
{
    ShaderProgramLinkingFailedException(const char* errorMessage) :
        std::runtime_error{ errorMessage }
    {
    }
};

class Texture;


class Shader
{
public:
    Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource);
    Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource, std::string_view geometryShaderSource);
    Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource, std::string_view geometryShaderSource, std::string_view tesselationControlShader, std::string_view tesselationEvaluateShader);

    static std::shared_ptr<Shader> LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
    static std::shared_ptr<Shader> LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, std::string_view geometryShaderPath);
    static std::shared_ptr<Shader> LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, std::string_view geometryShaderPath, std::string_view tesselationControlShaderPath, std::string_view tesselationEvaluateShaderPath);

    Shader(Shader&& shader) noexcept;
    Shader& operator=(Shader&& shader) noexcept;

    ~Shader();

public:
    void Use() const;
    void StopUsing() const;

    void SetUniformInt(const char* name, int value);
    void SetUniformFloat(const char* name, float value);
    void SetUniformVec2(const char* name, glm::vec2 value);
    void SetUniformVec3(const char* name, const glm::vec3& value);
    void SetUniformVec4(const char* name, const glm::vec4& value);

    void SetUniformMat4(const char* name, const glm::mat4& value);
    void SetUniformMat3(const char* name, const glm::mat3& value);

    int GetUniformInt(const char* name) const;

    float GetUniformFloat(const char* name) const;
    glm::vec2 GetUniformVec2(const char* name) const;
    glm::vec3 GetUniformVec3(const char* name) const;
    glm::vec4 GetUniformVec4(const char* name) const;

    void GetUniformInfos(std::vector<UniformInfo>& outUniformInfos) const;

    void SetSamplerUniform(const char* uniformName, const Texture& texture, unsigned int textureUnit);

private:
    GLuint _shaderProgram;
    mutable std::unordered_map<std::string, int> _uniformLocationsCache;

private:
    int GetUniformLocation(const char* uniformName) const;
    void AddNewUniformInfo(std::vector<UniformInfo>& outUniformInfos, GLint location) const;
};