#include "opengl_shader.h"
#include "logging.h"
#include "texture.h"

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <vector>

#include <sstream>
#include <array>
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
        GLenum GlUniformType;
        UniformType Type;
    };

    /* Wraps shader object with RAII object */
    struct ShaderObject
    {
        GLuint ShaderObj = 0;
        GLuint ShaderProgram = 0;

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
            ShaderProgram = tempObject.ShaderProgram;
            tempObject.ShaderObj = 0;
            tempObject.ShaderProgram = 0;
            return *this;
        }

        ~ShaderObject()
        {
            if (ShaderProgram != 0)
            {
                glDetachShader(ShaderProgram, ShaderObj);
            }
            else if (ShaderObj != 0)
            {
                glDeleteShader(ShaderObj);
            }
        }

        void AttachShaderToProgram(GLuint program)
        {
            this->ShaderProgram = program;
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
        int32_t logLength;

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

    ShaderObject TryCompileShader(const char* shaderSource, int32_t length, GLenum type)
    {
        GLuint shaderObject = glCreateShader(type);

        glShaderSource(shaderObject, 1, &shaderSource, &length);
        glCompileShader(shaderObject);

        int32_t compiledSuccesfully;

        glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &compiledSuccesfully);

        if (compiledSuccesfully == GL_FALSE)
        {
            ThrowShaderCompilationError(shaderObject, type);
        }

        return ShaderObject{shaderObject};
    }

    void ThrowLinkingError(GLuint program)
    {
        std::vector<char> log;

        int32_t logLength;

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

OpenGlShader::OpenGlShader() :
    m_ShaderProgram{0}
{
}

OpenGlShader::OpenGlShader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource)
{
    std::array<std::string_view, 2> sources{vertexShaderSource, fragmentShaderSource};
    GenerateShaders(sources);
}

OpenGlShader::OpenGlShader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource,
    std::string_view geometryShaderSource)
{
    std::array<std::string_view, 3> sources{vertexShaderSource, fragmentShaderSource, geometryShaderSource};
    GenerateShaders(sources);
}

OpenGlShader::OpenGlShader(std::string_view vertexShaderSource, std::string_view fragmentShaderSource,
    std::string_view geometryShaderSource, std::string_view tesselationControlShader,
    std::string_view tesselationEvaluateShader)
{
    std::array<std::string_view, 5> sources{vertexShaderSource, fragmentShaderSource,
        geometryShaderSource, tesselationControlShader, tesselationEvaluateShader};

    GenerateShaders(sources);
}

OpenGlShader::~OpenGlShader()
{
    glDeleteProgram(m_ShaderProgram);
}

void OpenGlShader::Use() const
{
    glUseProgram(m_ShaderProgram);
}

void OpenGlShader::StopUsing() const
{
    glUseProgram(0);
}

void OpenGlShader::SetUniformInt(const char* name, int32_t  value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void OpenGlShader::SetUniformFloat(const char* name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void OpenGlShader::SetUniformVec2(const char* name, glm::vec2 value)
{
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void OpenGlShader::SetUniformVec3(const char* name, const glm::vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void OpenGlShader::SetUniformVec4(const char* name, const glm::vec4& value)
{
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void OpenGlShader::SetUniformMat4(const char* name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGlShader::SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, int32_t count)
{
    glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, glm::value_ptr(values[0]));
}

void OpenGlShader::SetUniformMat3(const char* name, const glm::mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

int32_t  OpenGlShader::GetUniformInt(const char* name) const
{
    int32_t value = 0;
    glGetUniformiv(m_ShaderProgram, GetUniformLocation(name), &value);
    return value;
}

float OpenGlShader::GetUniformFloat(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value[0];
}

glm::vec2 OpenGlShader::GetUniformVec2(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec3 OpenGlShader::GetUniformVec3(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec4 OpenGlShader::GetUniformVec4(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(m_ShaderProgram, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

std::vector<UniformInfo> OpenGlShader::GetUniformInfos() const
{
    GLint numUniforms;

    glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::vector<UniformInfo> uniformsInfo;
    uniformsInfo.reserve(numUniforms);

    for (GLint uniformLocation = 0; uniformLocation < numUniforms; uniformLocation++)
    {
        AddNewUniformInfo(uniformsInfo, uniformLocation);
    }

    return uniformsInfo;
}

void OpenGlShader::SetSamplerUniform(const char* uniformName, const std::shared_ptr<ITexture>& textures, uint32_t startTextureUnit)
{

    ASSERT(startTextureUnit < kMinTextureUnits);
    glUniform1i(GetUniformLocation(uniformName), static_cast<GLint>(startTextureUnit));
}


void OpenGlShader::GenerateShaders(std::span<std::string_view> sources)
{
    GLenum types[ShaderIndex::kCount] = {GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER};

    std::size_t shaderIndex = 0;
    std::array<ShaderObject, ShaderIndex::kCount> shaders;

    for (const std::string_view& source : sources)
    {
        shaders[shaderIndex] = TryCompileShader(source.data(), static_cast<int32_t>(source.length()), types[shaderIndex]);
        shaderIndex++;
    }

    m_ShaderProgram = glCreateProgram();

    for (std::size_t i = 0; i < shaderIndex; ++i)
    {
        shaders[i].AttachShaderToProgram(m_ShaderProgram);
    }

    glLinkProgram(m_ShaderProgram);

    int32_t linkedSuccesfully;

    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &linkedSuccesfully);

    if (linkedSuccesfully == GL_FALSE)
    {
        ThrowLinkingError(m_ShaderProgram);
    }
}

int32_t OpenGlShader::GetUniformLocation(const char* uniformName) const
{
    auto it = m_UniformNameToLocation.find(uniformName);

    if (it == m_UniformNameToLocation.end())
    {
        int32_t location = glGetUniformLocation(m_ShaderProgram, uniformName);
        m_UniformNameToLocation[uniformName] = location;

        return location;
    }

    return it->second;
}

void OpenGlShader::AddNewUniformInfo(std::vector<UniformInfo>& outUniformsInfo, int32_t location) const
{
    const int32_t kMaxNameLength = 96;
    const GLTypeToUniformType kGLTypesToUniformTypes[] =
    {
        {GL_FLOAT, UniformType::kFloat},
        {GL_INT, UniformType::kInt},
        {GL_FLOAT_VEC2, UniformType::kVec2},
        {GL_FLOAT_VEC3, UniformType::kVec3},
        {GL_FLOAT_VEC4, UniformType::kVec4},
        {GL_FLOAT_MAT4, UniformType::kMat4x4},
        {GL_FLOAT_MAT3, UniformType::kMat3x3},
        {GL_BOOL, UniformType::kBoolean},
        {GL_INT_VEC2, UniformType::kIvec2},
        {GL_INT_VEC3, UniformType::kIvec3},
        {GL_SAMPLER_2D, UniformType::kSampler2D},
    };

    GLchar name[kMaxNameLength]; // variable name in GLSL
    GLsizei nameLength; // name length
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    glGetActiveUniform(m_ShaderProgram, static_cast<GLuint>(location), kMaxNameLength, &nameLength, &size, &type, name);
    auto it = std::find_if(std::begin(kGLTypesToUniformTypes), std::end(kGLTypesToUniformTypes),
        [&](const GLTypeToUniformType& t) { return t.GlUniformType == type; });

    if (it != std::end(kGLTypesToUniformTypes))
    {
        outUniformsInfo.emplace_back(UniformInfo{it->Type, name, location, static_cast<int32_t>(size)});
    }
}
