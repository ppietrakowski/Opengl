#include "Shader.h"

#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <array>

#include <glm/gtc/type_ptr.hpp>
#include "Logging.h"

#include "Texture.h"

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
        GLenum GLUniformType;
        UniformType Type;
    };

    /* Wraps shader object with RAII object */
    struct ShaderObject
    {
        GLuint GLShader = 0;
        GLuint Program = 0;

        ShaderObject() = default;
        ShaderObject(GLuint object) :
            GLShader{ object }
        {
        }

        ShaderObject(ShaderObject&& tempObject) noexcept
        {
            *this = std::move(tempObject);
        }

        ShaderObject& operator=(ShaderObject&& tempObject) noexcept
        {
            GLShader = tempObject.GLShader;
            Program = tempObject.Program;
            tempObject.GLShader = 0;
            tempObject.Program = 0;
            return *this;
        }

        ~ShaderObject()
        {
            if (Program != 0)
            {
                glDetachShader(Program, GLShader);
            }
            else if (GLShader != 0)
            {
                glDeleteShader(GLShader);
            }
        }

        void AttachShaderToProgram(GLuint program)
        {
            Program = program;
            glAttachShader(program, GLShader);
        }

        bool IsValid() const { return GLShader != 0; }
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

        msg += " compilation failure: ";
        msg += log.data();

        ELOG_ERROR(LOG_RENDERER, "%s", msg.c_str());
        throw ShaderCompilationFailedException(msg.c_str());
    }

    ShaderObject TryCompileShader(const char* shaderSource, std::int32_t length, GLenum type)
    {
        GLuint shaderObj = glCreateShader(type);

        glShaderSource(shaderObj, 1, &shaderSource, &length);
        glCompileShader(shaderObj);

        std::int32_t compiledWithoutErrors;

        glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &compiledWithoutErrors);

        if (compiledWithoutErrors == GL_FALSE)
        {
            ThrowShaderCompilationError(shaderObj, type);
        }

        return ShaderObject{ shaderObj };
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

    std::string LoadFileContent(const std::string& filePath)
    {
        std::ifstream file(filePath.c_str());
        file.exceptions(std::ios::failbit | std::ios::badbit);

        std::ostringstream content;

        content << file.rdbuf();

        return content.str();
    }
}

Shader::Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource)
{
    std::array<std::string_view, 2> sources{ vertexShaderSource, fragmentShaderSource };
    GenerateShaders(sources);
}

Shader::Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource,
    std::string_view geometryShaderSource)
{
    std::array<std::string_view, 3> sources{ vertexShaderSource, fragmentShaderSource, geometryShaderSource };
    GenerateShaders(sources);
}

Shader::Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource,
    std::string_view geometryShaderSource, std::string_view tesselationControlShaderSource,
    std::string_view tesselationEvaluateShaderSource)
{
    std::array<std::string_view, 5> sources{ vertexShaderSource, fragmentShaderSource,
        geometryShaderSource, tesselationControlShaderSource, tesselationEvaluateShaderSource };

    GenerateShaders(sources);
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
{
    return LoadShader({ vertexShaderPath, fragmentShaderPath });
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath,
    std::string_view geometryShaderPath)
{
    return LoadShader({ vertexShaderPath, fragmentShaderPath, geometryShaderPath });
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath,
    std::string_view geometryShaderPath, std::string_view tesselationControlShaderPath, std::string_view tesselationEvaluateShaderPath)
{
    return LoadShader({ vertexShaderPath, fragmentShaderPath, geometryShaderPath, tesselationControlShaderPath, tesselationEvaluateShaderPath });
}

Shader::Shader(Shader&& shader) noexcept
{
    *this = std::move(shader);
}

Shader& Shader::operator=(Shader&& shader) noexcept
{
    std::swap(shader._shaderProgram, _shaderProgram);
    _uniformLocationsCache = std::move(_uniformLocationsCache);
    return *this;
}

Shader::~Shader()
{
    ELOG_VERBOSE(LOG_RENDERER, "Removing shader with ProgramID=%i", _shaderProgram);
    glDeleteProgram(_shaderProgram);
}

void Shader::Use() const
{
    glUseProgram(_shaderProgram);
}

void Shader::StopUsing() const
{
    glUseProgram(0);
}

void Shader::SetUniformInt(const char* name, std::int32_t  value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniformFloat(const char* name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniformVec2(const char* name, glm::vec2 value)
{
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniformVec3(const char* name, const glm::vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniformVec4(const char* name, const glm::vec4& value)
{
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniformMat4(const char* name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniformMat3(const char* name, const glm::mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

std::int32_t  Shader::GetUniformInt(const char* name) const
{
    std::int32_t value = 0;
    glGetUniformiv(_shaderProgram, GetUniformLocation(name), &value);
    return value;
}

float Shader::GetUniformFloat(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(_shaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value[0];
}

glm::vec2 Shader::GetUniformVec2(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(_shaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec3 Shader::GetUniformVec3(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(_shaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec4 Shader::GetUniformVec4(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(_shaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

void Shader::GetUniformInfos(std::vector<UniformInfo>& outUniformInfo) const
{
    GLint numUniforms;

    glGetProgramiv(_shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);

    outUniformInfo.reserve(numUniforms);

    for (GLint uniformLocation = 0; uniformLocation < numUniforms; uniformLocation++)
    {
        AddNewUniformInfo(outUniformInfo, uniformLocation);
    }
}

void Shader::SetSamplerUniform(const char* uniformName, const Texture& texture, std::uint32_t TextureUnit)
{
    texture.Bind(TextureUnit);
    SetUniformInt(uniformName, static_cast<std::int32_t>(TextureUnit));
}

std::shared_ptr<Shader> Shader::LoadShader(const std::initializer_list<std::string_view>& paths)
{
    std::array<std::string, ShaderIndex::Count> sources;
    std::size_t index = 0;

    auto it = sources.begin();

    for (const std::string_view& path : paths)
    {
        sources[index++] = LoadFileContent(std::string{ path.begin(), path.end() });
        ++it;
    }

    std::array<std::string_view, ShaderIndex::Count> convertexToStringViewSources;
    std::transform(sources.begin(), it, convertexToStringViewSources.begin(),
        [](const std::string& s) { return (std::string_view)s; });

    std::shared_ptr<Shader> shader = std::make_shared<Shader>();
    shader->GenerateShaders(std::span<std::string_view>{ convertexToStringViewSources.begin(), index });
    return shader;
}

void Shader::GenerateShaders(std::span<std::string_view> sources)
{
    GLenum types[ShaderIndex::Count] = { GL_VERTEX_SHADER, 
        GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER };

    std::size_t shaderIndex = 0;
    std::array<ShaderObject, ShaderIndex::Count> shaders;

    for (const std::string_view& source : sources)
    {
        shaders[shaderIndex] = TryCompileShader(source.data(), static_cast<std::int32_t>(source.length()), types[shaderIndex]);
        shaderIndex++;
    }

    _shaderProgram = glCreateProgram();

    for (std::size_t i = 0; i < shaderIndex; ++i)
    {
        shaders[i].AttachShaderToProgram(_shaderProgram);
    }

    glLinkProgram(_shaderProgram);

    std::int32_t linkedWithoutErrors;

    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &linkedWithoutErrors);

    if (linkedWithoutErrors == GL_FALSE)
    {
        ThrowLinkingError(_shaderProgram);
    }
}

std::int32_t Shader::GetUniformLocation(const char* uniformName) const
{
    auto it = _uniformLocationsCache.find(uniformName);

    if (it == _uniformLocationsCache.end())
    {
        std::int32_t location = glGetUniformLocation(_shaderProgram, uniformName);
        _uniformLocationsCache[uniformName] = location;

        return location;
    }

    return it->second;
}

void Shader::AddNewUniformInfo(std::vector<UniformInfo>& outUniformInfos, GLint location) const
{
    const std::uint32_t MaxNameLength = 96;
    GLTypeToUniformType GLTypesToUniformTypes[] =
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

    glGetActiveUniform(_shaderProgram, static_cast<GLuint>(location), MaxNameLength, &nameLength, &size, &type, name);
    auto it = std::find_if(std::begin(GLTypesToUniformTypes), std::end(GLTypesToUniformTypes),
        [&](GLTypeToUniformType& t) { return t.GLUniformType == type; });

    if (it != std::end(GLTypesToUniformTypes))
    {
        outUniformInfos.push_back(UniformInfo{ it->Type, name, location });
    }
}
