#include "Shader.h"

#include <string>
#include <fstream>
#include <vector>
#include <sstream>

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
        EUniformType Type;
    };

    /* Wraps shader object with RAII object */
    struct ShaderObject
    {
        GLuint GLShader = 0;
        GLuint Program = 0;

        ShaderObject() = default;
        ShaderObject(GLuint object) :
            GLShader(object)
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

        int logLength;

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

    ShaderObject TryCompileShader(const char* shaderSource, int length, GLenum type)
    {
        GLuint shaderObj = glCreateShader(type);

        glShaderSource(shaderObj, 1, &shaderSource, &length);
        glCompileShader(shaderObj);

        int compiledWithoutErrors;

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

        int logLength;

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
    ShaderObject vertexShader = TryCompileShader(vertexShaderSource.data(),
        static_cast<int>(vertexShaderSource.length()), GL_VERTEX_SHADER);

    ShaderObject fragmentShader = TryCompileShader(fragmentShaderSource.data(),
        static_cast<int>(fragmentShaderSource.length()), GL_FRAGMENT_SHADER);

    _shaderProgram = glCreateProgram();
    vertexShader.AttachShaderToProgram(_shaderProgram);
    fragmentShader.AttachShaderToProgram(_shaderProgram);
    glLinkProgram(_shaderProgram);

    int linkedWithoutErrors;

    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &linkedWithoutErrors);

    if (linkedWithoutErrors == GL_FALSE)
    {
        ThrowLinkingError(_shaderProgram);
    }
}

Shader::Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource, std::string_view geometryShaderSource)
{
    ShaderObject vertexShader = TryCompileShader(vertexShaderSource.data(),
        static_cast<int>(vertexShaderSource.length()), GL_VERTEX_SHADER);

    ShaderObject fragmentShader = TryCompileShader(fragmentShaderSource.data(),
        static_cast<int>(fragmentShaderSource.length()), GL_FRAGMENT_SHADER);

    ShaderObject geometryShader = TryCompileShader(geometryShaderSource.data(),
        static_cast<int>(geometryShaderSource.length()), GL_GEOMETRY_SHADER);

    _shaderProgram = glCreateProgram();
    vertexShader.AttachShaderToProgram(_shaderProgram);
    fragmentShader.AttachShaderToProgram(_shaderProgram);
    geometryShader.AttachShaderToProgram(_shaderProgram);

    glLinkProgram(_shaderProgram);

    int linkedWithoutErrors;

    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &linkedWithoutErrors);

    if (linkedWithoutErrors == GL_FALSE)
    {
        ThrowLinkingError(_shaderProgram);
    }
}

Shader::Shader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource, 
    std::string_view geometryShaderSource, std::string_view tesselationControlShaderSource,
    std::string_view tesselationEvaluateShaderSource)
{
    ShaderObject vertexShader = TryCompileShader(vertexShaderSource.data(),
        static_cast<int>(vertexShaderSource.length()), GL_VERTEX_SHADER);

    ShaderObject fragmentShader = TryCompileShader(fragmentShaderSource.data(),
        static_cast<int>(fragmentShaderSource.length()), GL_FRAGMENT_SHADER);

    ShaderObject geometryShader = TryCompileShader(geometryShaderSource.data(),
        static_cast<int>(geometryShaderSource.length()), GL_GEOMETRY_SHADER);

    ShaderObject tesselationControlShader = TryCompileShader(tesselationControlShaderSource.data(),
        static_cast<int>(tesselationControlShaderSource.length()), GL_TESS_CONTROL_SHADER);

    ShaderObject tesselationEvaluationShader = TryCompileShader(tesselationEvaluateShaderSource.data(),
        static_cast<int>(tesselationEvaluateShaderSource.length()), GL_TESS_EVALUATION_SHADER);

    _shaderProgram = glCreateProgram();

    vertexShader.AttachShaderToProgram(_shaderProgram);
    fragmentShader.AttachShaderToProgram(_shaderProgram);
    geometryShader.AttachShaderToProgram(_shaderProgram);
    tesselationEvaluationShader.AttachShaderToProgram(_shaderProgram);
    tesselationControlShader.AttachShaderToProgram(_shaderProgram);

    glLinkProgram(_shaderProgram);

    int linkedWithoutErrors;

    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &linkedWithoutErrors);

    if (linkedWithoutErrors == GL_FALSE)
    {
        ThrowLinkingError(_shaderProgram);
    }
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
{
    std::string vertexSource = LoadFileContent(vertexShaderPath.data());
    std::string fragmentSource = LoadFileContent(fragmentShaderPath.data());

    return std::make_shared<Shader>(vertexSource, fragmentSource);
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, std::string_view geometryShaderPath)
{
    std::string vertexSource = LoadFileContent(vertexShaderPath.data());
    std::string fragmentSource = LoadFileContent(fragmentShaderPath.data());
    std::string geometrySource = LoadFileContent(geometryShaderPath.data());

    return std::make_shared<Shader>(vertexSource, fragmentSource, geometrySource);
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, std::string_view geometryShaderPath, std::string_view tesselationControlShaderPath, std::string_view tesselationEvaluateShaderPath)
{
    std::string vertexSource = LoadFileContent(vertexShaderPath.data());
    std::string fragmentSource = LoadFileContent(fragmentShaderPath.data());
    std::string geometrySource = LoadFileContent(geometryShaderPath.data());
    std::string tessControlSource = LoadFileContent(tesselationControlShaderPath.data());
    std::string tessEvaluateSource = LoadFileContent(tesselationEvaluateShaderPath.data());

    return std::make_shared<Shader>(vertexSource, fragmentSource, geometrySource, tessControlSource, tessEvaluateSource);
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

void Shader::SetUniformInt(const char* name, int value)
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

int Shader::GetUniformInt(const char* name) const
{
    int value;
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

void Shader::SetSamplerUniform(const char* uniformName, const Texture& texture, unsigned int textureUnit)
{
    texture.Bind(textureUnit);
    SetUniformInt(uniformName, static_cast<int>(textureUnit));
}

int Shader::GetUniformLocation(const char* uniformName) const
{
    auto it = _uniformLocationsCache.find(uniformName);

    if (it == _uniformLocationsCache.end())
    {
        int location = glGetUniformLocation(_shaderProgram, uniformName);
        _uniformLocationsCache[uniformName] = location;

        return location;
    }

    return it->second;
}

void Shader::AddNewUniformInfo(std::vector<UniformInfo>& outUniformInfos, GLint location) const
{
    const unsigned int MaxNameLength = 96;
    GLTypeToUniformType GLTypesToUniformTypes[] =
    {
        {GL_FLOAT, EUniformType::Float},
        {GL_INT, EUniformType::Int},
        {GL_FLOAT_VEC2, EUniformType::Vec2},
        {GL_FLOAT_VEC3, EUniformType::Vec3},
        {GL_FLOAT_VEC4, EUniformType::Vec4},
        {GL_FLOAT_MAT4, EUniformType::Mat4x4},
        {GL_FLOAT_MAT3, EUniformType::Mat3x3},
        {GL_BOOL, EUniformType::Boolean},
        {GL_INT_VEC2, EUniformType::Ivec2},
        {GL_INT_VEC3, EUniformType::Ivec3},
        {GL_SAMPLER_2D, EUniformType::Sampler2D},
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
