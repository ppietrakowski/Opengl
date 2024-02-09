#include "Shader.hpp"

#include "RendererApi.hpp"
#include "ErrorMacros.hpp"

#include <sstream>
#include <array>
#include <fstream>
#include <algorithm>

#include "Logging.hpp"
#include "Texture.hpp"

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
        GLenum GlUniformType;
        UniformType Type;
    };

    /* Wraps shader object with RAII object */
    struct ShaderObject
    {
        GLuint Object = 0;
        GLuint AttachedProgram = 0;

        ShaderObject() = default;
        ShaderObject(GLuint object) :
            Object{object}
        {
        }

        ShaderObject(ShaderObject&& tempObject) noexcept
        {
            *this = std::move(tempObject);
        }

        ShaderObject& operator=(ShaderObject&& tempObject) noexcept
        {
            Object = tempObject.Object;
            AttachedProgram = tempObject.AttachedProgram;
            tempObject.Object = 0;
            tempObject.AttachedProgram = 0;
            return *this;
        }

        ~ShaderObject()
        {
            if (AttachedProgram != 0)
            {
                glDetachShader(AttachedProgram, Object);
            }
            else if (Object != 0)
            {
                glDeleteShader(Object);
            }
        }

        void AttachShaderToProgram(GLuint program)
        {
            AttachedProgram = program;
            glAttachShader(program, Object);
        }

        bool IsValid() const
        {
            return Object != 0;
        }
    };

    void ThrowShaderCompilationError(GLuint shader, GLenum type)
    {
        GLint logLength;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> log(logLength, 0);
        glGetShaderInfoLog(shader, logLength, &logLength, log.data());

        ENG_LOG_ERROR("{} shader compilation failure: {}", ShaderTypeToString(type), log.data());
        throw ShaderCompilationFailedException(log.data());
    }

    ShaderObject TryCompileShader(const char* shaderSource, GLint length, GLenum type)
    {
        GLuint shaderObject = glCreateShader(type);

        glShaderSource(shaderObject, 1, &shaderSource, &length);
        glCompileShader(shaderObject);

        GLint bCompiledSuccesfully;

        glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &bCompiledSuccesfully);

        if (bCompiledSuccesfully == GL_FALSE)
        {
            ThrowShaderCompilationError(shaderObject, type);
        }

        return ShaderObject{shaderObject};
    }

    void ThrowLinkingError(GLuint program)
    {
        GLint logLength;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> log(logLength, 0);
        glGetProgramInfoLog(program, logLength, &logLength, log.data());

        ENG_LOG_ERROR("linking failure: {}", log.data());

        glDeleteProgram(program);
        throw ShaderProgramLinkingFailedException(log.data());
    }
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

void Shader::SetUniform(const char* name, int32_t value)
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

void Shader::SetUniformMat4Array(const char* name, std::span<const glm::mat4> values)
{
    glUniformMatrix4fv(GetUniformLocation(name),
        static_cast<GLsizei>(values.size()), GL_FALSE, glm::value_ptr(values[0]));
}

void Shader::SetUniform(const char* name, const glm::mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
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

void Shader::SetSamplersUniform(const char* uniformName, std::span<const std::shared_ptr<Texture>> textures, uint32_t startTextureUnit)
{

    std::array<int32_t, MinTextureUnits> textureUnits;

    int32_t last = 0;
    std::generate(textureUnits.begin(), textureUnits.begin() + textures.size(), [&last]()
    {
        return last++;
    });

    glUniform1iv(GetUniformLocation(uniformName), static_cast<GLsizei>(textures.size()), textureUnits.data());
}

void Shader::BindUniformBuffer(int32_t blockIndex, const UniformBuffer& buffer)
{
    buffer.Bind(blockIndex);
    glUniformBlockBinding(m_ShaderProgram, blockIndex, blockIndex);
}

int32_t Shader::GetUniformBlockIndex(const std::string& name) const
{
    auto it = m_UniformNameToLocation.find(name);

    if (it == m_UniformNameToLocation.end())
    {
        GLint location = glGetUniformBlockIndex(m_ShaderProgram, name.c_str());
        m_UniformNameToLocation[name] = location;
        return location;
    }

    return it->second;
}

void Shader::GenerateShaders(std::span<std::string_view> sources)
{
    GLenum types[ShaderIndex::Count] = {GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER};

    size_t shaderIndex = 0;
    std::array<ShaderObject, ShaderIndex::Count> shaders;

    for (const std::string_view& source : sources)
    {
        shaders[shaderIndex] = TryCompileShader(source.data(), static_cast<GLint>(source.length()), types[shaderIndex]);
        shaderIndex++;
    }

    m_ShaderProgram = glCreateProgram();

    for (size_t i = 0; i < shaderIndex; ++i)
    {
        shaders[i].AttachShaderToProgram(m_ShaderProgram);
    }

    glLinkProgram(m_ShaderProgram);

    GLint bLinkedSuccesfully;

    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &bLinkedSuccesfully);

    if (bLinkedSuccesfully == GL_FALSE)
    {
        ThrowLinkingError(m_ShaderProgram);
    }
}

void Shader::GenerateShaders(std::span<const std::string> sources)
{
    ASSERT(sources.size() < ShaderIndex::Count);
    std::array<std::string_view, ShaderIndex::Count> srcs;
    uint32_t index = 0;

    for (const std::string& src : sources)
    {
        srcs[index++] = src;
    }

    GenerateShaders(std::span<std::string_view>{srcs.begin(), index});
}

int32_t Shader::GetUniformLocation(const char* uniformName) const
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

void Shader::AddNewUniformInfo(std::vector<UniformInfo>& outUniformsInfo, int32_t location) const
{
    const int32_t MaxNameLength = 96;
    const GLTypeToUniformType GlTypesToUniformTypes[] =
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
    auto it = std::find_if(std::begin(GlTypesToUniformTypes), std::end(GlTypesToUniformTypes),
        [&](const GLTypeToUniformType& t)
    {
        return t.GlUniformType == type;
    });

    if (it != std::end(GlTypesToUniformTypes))
    {
        outUniformsInfo.emplace_back(UniformInfo{it->Type, name, location, static_cast<int32_t>(size)});
    }
}

