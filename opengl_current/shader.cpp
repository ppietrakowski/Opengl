#include "shader.h"

#include "renderer_api.h"
#include "error_macros.h"

#include <sstream>
#include <array>
#include <fstream>
#include <algorithm>

#include "logging.h"
#include "texture.h"

#include <GL/glew.h>
#include <string>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace
{
    const char* ShaderTypeToString(GLenum type)
    {
        switch (type)
        {
        case GL_VERTEX_SHADER:
            return "vertex";
        case GL_FRAGMENT_SHADER:
            return "fragment";
        case GL_GEOMETRY_SHADER:
            return "geometry";
        case GL_TESS_EVALUATION_SHADER:
            return "tesselation-evaluation";
        case GL_TESS_CONTROL_SHADER:
            return "tesselation-control";
        }

        return "???";
    }

    struct GLTypeToUniformType
    {
        GLenum OpenGlUniformType;
        UniformType Type;
    };

    /* Wraps shader object with RAII object */
    struct ShaderObject
    {
        GLuint ShaderObj = 0;
        GLuint Program = 0;

        ShaderObject() = default;
        ShaderObject(GLuint object) :
            ShaderObj{object}
        {
        }

        ShaderObject(ShaderObject&& tempObject) noexcept
        {
            *this = std::move(tempObject);
        }

        ShaderObject& operator=(ShaderObject&& tempObject) noexcept
        {
            ShaderObj = tempObject.ShaderObj;
            Program = tempObject.Program;
            tempObject.ShaderObj = 0;
            tempObject.Program = 0;
            return *this;
        }

        ~ShaderObject()
        {
            if (Program != 0)
            {
                glDetachShader(Program, ShaderObj);
            }
            else if (ShaderObj != 0)
            {
                glDeleteShader(ShaderObj);
            }
        }

        void AttachShaderToProgram(GLuint program)
        {
            Program = program;
            glAttachShader(program, ShaderObj);
        }

        bool IsValid() const
        {
            return ShaderObj != 0;
        }
    };

    void ThrowShaderCompilationError(GLuint shader, GLenum type)
    {
        std::vector<char> log;
        std::int32_t logLength;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        log.resize(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log.data());

        std::string msg;

        msg = ShaderTypeToString(type);

        msg += " shader compilation failure: ";
        msg += log.data();

        ELOG_ERROR(LOG_RENDERER, "%s", msg.c_str());
        throw ShaderCompilationFailedException(msg.c_str());
    }

    ShaderObject TryCompileShader(const char* shaderSource, std::int32_t length, GLenum type)
    {
        GLuint shaderObject = glCreateShader(type);

        glShaderSource(shaderObject, 1, &shaderSource, &length);
        glCompileShader(shaderObject);

        std::int32_t bCompiledSuccesfully;

        glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &bCompiledSuccesfully);

        if (bCompiledSuccesfully == GL_FALSE)
        {
            ThrowShaderCompilationError(shaderObject, type);
        }

        return ShaderObject{shaderObject};
    }

    void ThrowLinkingError(GLuint program)
    {
        std::vector<char> log;

        std::int32_t logLength;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        log.resize(logLength + 1);
        log[logLength] = 0;

        glGetProgramInfoLog(program, logLength, &logLength, log.data());

        std::string msg;
        msg += "linking failure: ";
        msg += log.data();
        ELOG_ERROR(LOG_RENDERER, "%s", msg.c_str());

        glDeleteProgram(program);
        throw ShaderProgramLinkingFailedException(msg.c_str());
    }
}

std::shared_ptr<Shader> ShaderSourceBuilder::Build()
{
    std::uint32_t index = GetLastShaderIndex();
    return std::make_shared<Shader>(std::span<const std::string>{m_ShaderSources.begin(), index});
}

ShaderSourceBuilder& ShaderSourceBuilder::SetVertexShaderSource(const std::string& source)
{
    m_ShaderSources[ShaderIndex::Vertex] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadVertexShaderSource(const std::filesystem::path& filePath)
{
    m_ShaderSources[ShaderIndex::Vertex] = LoadFileContent(filePath);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetFragmentShaderSource(const std::string& source)
{
    m_ShaderSources[ShaderIndex::Fragment] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadFragmentShaderSource(const std::filesystem::path& filePath)
{
    m_ShaderSources[ShaderIndex::Fragment] = LoadFileContent(filePath);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetGeometryShaderSource(const std::string& source)
{
    m_ShaderSources[ShaderIndex::Geometry] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadGeometryShaderSource(const std::filesystem::path& filePath)
{
    m_ShaderSources[ShaderIndex::Geometry] = LoadFileContent(filePath);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetTesselationShaderSource(const std::string& controlShaderSource,
    const std::string& evaluateShaderSource)
{
    m_ShaderSources[ShaderIndex::TesselationControlShader] = controlShaderSource;
    m_ShaderSources[ShaderIndex::TesselationEvaluateShader] = evaluateShaderSource;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadGeometryShaderSource(const std::filesystem::path& controlShaderPath,
    const std::filesystem::path& evaluateShaderPath)
{
    m_ShaderSources[ShaderIndex::TesselationControlShader] = LoadFileContent(controlShaderPath);
    m_ShaderSources[ShaderIndex::TesselationEvaluateShader] = LoadFileContent(evaluateShaderPath);
    return *this;
}

std::uint32_t ShaderSourceBuilder::GetLastShaderIndex() const
{
    for (auto it = m_ShaderSources.rbegin(); it != m_ShaderSources.rend(); ++it)
    {
        if (it->empty())
        {
            return static_cast<std::uint32_t>(std::distance(m_ShaderSources.rbegin(), it));
        }
    }

    return ShaderIndex::Count;
}

Shader::Shader(std::span<const std::string> sources)
{
    GenerateShaders(sources);
}

Shader::~Shader()
{
    glDeleteProgram(m_ShaderProgram);
}

void Shader::Use() const
{
    glUseProgram(m_ShaderProgram);
}

void Shader::StopUsing() const
{
    glUseProgram(0);
}

void Shader::SetUniform(const char* name, std::int32_t  value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform(const char* name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform(const char* name, glm::vec2 value)
{
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char* name, const glm::vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char* name, const glm::vec4& value)
{
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniform(const char* name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, std::uint32_t count)
{
    glUniformMatrix4fv(GetUniformLocation(name), static_cast<GLsizei>(count), GL_FALSE, glm::value_ptr(values[0]));
}

void Shader::SetUniform(const char* name, const glm::mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

std::int32_t Shader::GetUniformInt(const char* name) const
{
    std::int32_t value = 0;
    glGetUniformiv(m_ShaderProgram, GetUniformLocation(name), &value);
    return value;
}

float Shader::GetUniformFloat(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value[0];
}

glm::vec2 Shader::GetUniformVec2(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec3 Shader::GetUniformVec3(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec4 Shader::GetUniformVec4(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

std::vector<UniformInfo> Shader::GetUniformsInfo() const
{
    GLint numUniforms;

    glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::vector<UniformInfo> uniformsInfo;
    uniformsInfo.reserve(numUniforms);

    for (GLint i = 0; i < numUniforms; i++)
    {
        AddNewUniformInfo(uniformsInfo, i);
    }

    return uniformsInfo;
}

void Shader::SetSamplerUniform(const char* uniformName, const std::shared_ptr<Texture>& textures, uint32_t startTextureUnit)
{
    ASSERT(startTextureUnit < MinTextureUnits);
    glUniform1i(GetUniformLocation(uniformName), static_cast<GLint>(startTextureUnit));
}


void Shader::GenerateShaders(std::span<std::string_view> sources)
{
    GLenum types[ShaderIndex::Count] = {GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER};

    std::size_t shaderIndex = 0;
    std::array<ShaderObject, ShaderIndex::Count> shaders;

    for (const std::string_view& source : sources)
    {
        shaders[shaderIndex] = TryCompileShader(source.data(), static_cast<std::int32_t>(source.length()), types[shaderIndex]);
        shaderIndex++;
    }

    m_ShaderProgram = glCreateProgram();

    for (std::size_t i = 0; i < shaderIndex; ++i)
    {
        shaders[i].AttachShaderToProgram(m_ShaderProgram);
    }

    glLinkProgram(m_ShaderProgram);

    std::int32_t linkedSuccesfully;

    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &linkedSuccesfully);

    if (linkedSuccesfully == GL_FALSE)
    {
        ThrowLinkingError(m_ShaderProgram);
    }
}

void Shader::GenerateShaders(std::span<const std::string> sources)
{
    ASSERT(sources.size() < ShaderIndex::Count);
    std::array<std::string_view, ShaderIndex::Count> srcs;
    std::uint32_t index = 0;

    for (const std::string& src : sources)
    {
        srcs[index++] = src;
    }

    GenerateShaders(std::span<std::string_view>{srcs.begin(), index});
}

GLint Shader::GetUniformLocation(const char* uniformName) const
{
    auto it = m_UniformNameToLocation.find(uniformName);

    if (it == m_UniformNameToLocation.end())
    {
        GLint location = glGetUniformLocation(m_ShaderProgram, uniformName);
        m_UniformNameToLocation[uniformName] = location;

        return location;
    }

    return it->second;
}

void Shader::AddNewUniformInfo(std::vector<UniformInfo>& outUniformsInfo, int location) const
{
    const std::int32_t MaxNameLength = 96;
    const GLTypeToUniformType GLTypesToUniformTypes[] =
    {
        {GL_FLOAT, UniformType::Float},
        {GL_INT, UniformType::Int},
        {GL_FLOAT_VEC2, UniformType::Vec2},
        {GL_FLOAT_VEC3, UniformType::Vec3},
        {GL_FLOAT_VEC4, UniformType::Vec4},
        {GL_FLOAT_MAT4, UniformType::Mat4x4},
        {GL_FLOAT_MAT3, UniformType::Mat3x3},
        {GL_BOOL, UniformType::Boolean},
        {GL_INT_VEC2, UniformType::Ivec2},
        {GL_INT_VEC3, UniformType::Ivec3},
        {GL_SAMPLER_2D, UniformType::Sampler2D},
    };

    GLchar name[MaxNameLength]; // variable name in GLSL
    GLsizei nameLength; // name length
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    glGetActiveUniform(m_ShaderProgram, static_cast<GLuint>(location), MaxNameLength, &nameLength, &size, &type, name);
    auto it = std::find_if(std::begin(GLTypesToUniformTypes), std::end(GLTypesToUniformTypes),
        [&](const GLTypeToUniformType& t) { return t.OpenGlUniformType == type; });

    if (it != std::end(GLTypesToUniformTypes))
    {
        outUniformsInfo.emplace_back(UniformInfo{it->Type, name, location, static_cast<std::int32_t>(size)});
    }
}

