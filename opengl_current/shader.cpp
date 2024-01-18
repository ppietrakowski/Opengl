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
        GLenum gl_uniform_type;
        UniformType uniform_type;
    };

    /* Wraps shader object with RAII object */
    struct ShaderObject
    {
        GLuint shader_object = 0;
        GLuint attached_program = 0;

        ShaderObject() = default;
        ShaderObject(GLuint object) :
            shader_object{object}
        {
        }

        ShaderObject(ShaderObject&& temp_object) noexcept
        {
            *this = std::move(temp_object);
        }

        ShaderObject& operator=(ShaderObject&& temp_object) noexcept
        {
            shader_object = temp_object.shader_object;
            attached_program = temp_object.attached_program;
            temp_object.shader_object = 0;
            temp_object.attached_program = 0;
            return *this;
        }

        ~ShaderObject()
        {
            if (attached_program != 0)
            {
                glDetachShader(attached_program, shader_object);
            }
            else if (shader_object != 0)
            {
                glDeleteShader(shader_object);
            }
        }

        void AttachShaderToProgram(GLuint program)
        {
            attached_program = program;
            glAttachShader(program, shader_object);
        }

        bool IsValid() const
        {
            return shader_object != 0;
        }
    };

    void ThrowShaderCompilationError(GLuint shader, GLenum type)
    {
        std::int32_t log_length;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> log(log_length, 0);
        glGetShaderInfoLog(shader, log_length, &log_length, log.data());

        std::string msg;

        msg = ShaderTypeToString(type);

        msg += " shader compilation failure: ";
        msg += log.data();

        ELOG_ERROR(LOG_RENDERER, "%s", msg.c_str());
        throw ShaderCompilationFailedException(msg.c_str());
    }

    ShaderObject TryCompileShader(const char* shader_source, std::int32_t length, GLenum type)
    {
        GLuint shader_object = glCreateShader(type);

        glShaderSource(shader_object, 1, &shader_source, &length);
        glCompileShader(shader_object);

        std::int32_t compiled_succesfully;

        glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compiled_succesfully);

        if (compiled_succesfully == GL_FALSE)
        {
            ThrowShaderCompilationError(shader_object, type);
        }

        return ShaderObject{shader_object};
    }

    void ThrowLinkingError(GLuint program)
    {

        std::int32_t log_length;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> log(log_length, 0);
        glGetProgramInfoLog(program, log_length, &log_length, log.data());

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
    return std::make_shared<Shader>(std::span<const std::string>{shader_sources_.begin(), index});
}

ShaderSourceBuilder& ShaderSourceBuilder::SetVertexShaderSource(const std::string& source)
{
    shader_sources_[ShaderIndex::kVertex] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadVertexShaderSource(const std::filesystem::path& file_path)
{
    shader_sources_[ShaderIndex::kVertex] = LoadFileContent(file_path);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetFragmentShaderSource(const std::string& source)
{
    shader_sources_[ShaderIndex::kFragment] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadFragmentShaderSource(const std::filesystem::path& file_path)
{
    shader_sources_[ShaderIndex::kFragment] = LoadFileContent(file_path);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetGeometryShaderSource(const std::string& source)
{
    shader_sources_[ShaderIndex::kGeometry] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadGeometryShaderSource(const std::filesystem::path& file_path)
{
    shader_sources_[ShaderIndex::kGeometry] = LoadFileContent(file_path);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetTesselationShaderSource(const std::string& control_shader_source,
    const std::string& evaluate_shader_source)
{
    shader_sources_[ShaderIndex::kTesselationControlShader] = control_shader_source;
    shader_sources_[ShaderIndex::kTesselationEvaluateShader] = evaluate_shader_source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadGeometryShaderSource(const std::filesystem::path& control_shader_path,
    const std::filesystem::path& evaluate_shader_path)
{
    shader_sources_[ShaderIndex::kTesselationControlShader] = LoadFileContent(control_shader_path);
    shader_sources_[ShaderIndex::kTesselationEvaluateShader] = LoadFileContent(evaluate_shader_path);
    return *this;
}

std::uint32_t ShaderSourceBuilder::GetLastShaderIndex() const
{
    for (auto it = shader_sources_.rbegin(); it != shader_sources_.rend(); ++it)
    {
        if (it->empty())
        {
            return static_cast<std::uint32_t>(std::distance(shader_sources_.rbegin(), it));
        }
    }

    return ShaderIndex::kCount;
}

Shader::Shader(std::span<const std::string> sources)
{
    GenerateShaders(sources);
}

Shader::~Shader()
{
    glDeleteProgram(shader_program_);
}

void Shader::Use() const
{
    glUseProgram(shader_program_);
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

void Shader::SetUniformMat4Array(const char* name, std::span<const glm::mat4> values)
{
    glUniformMatrix4fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), GL_FALSE, glm::value_ptr(values[0]));
}

void Shader::SetUniform(const char* name, const glm::mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

std::int32_t Shader::GetUniformInt(const char* name) const
{
    std::int32_t value = 0;
    glGetUniformiv(shader_program_, GetUniformLocation(name), &value);
    return value;
}

float Shader::GetUniformFloat(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value[0];
}

glm::vec2 Shader::GetUniformVec2(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec3 Shader::GetUniformVec3(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec4 Shader::GetUniformVec4(const char* name) const
{
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

std::vector<UniformInfo> Shader::GetUniformsInfo() const
{
    GLint num_uniforms;

    glGetProgramiv(shader_program_, GL_ACTIVE_UNIFORMS, &num_uniforms);
    std::vector<UniformInfo> uniforms_info;
    uniforms_info.reserve(num_uniforms);

    for (GLint i = 0; i < num_uniforms; i++)
    {
        AddNewUniformInfo(uniforms_info, i);
    }

    return uniforms_info;
}

void Shader::SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, uint32_t start_texture_unit)
{
    ASSERT(start_texture_unit < kMinTextureUnits);
    glUniform1i(GetUniformLocation(uniform_name), static_cast<GLint>(start_texture_unit));
}

void Shader::BindUniformBuffer(std::int32_t block_index, const UniformBuffer& buffer)
{
    buffer.Bind(block_index);
    glUniformBlockBinding(shader_program_, block_index, block_index);
}

int Shader::GetUniformBlockIndex(const std::string& name) const
{
    auto it = uniform_name_to_location_.find(name);

    if (it == uniform_name_to_location_.end())
    {
        GLint location = glGetUniformBlockIndex(shader_program_, name.c_str());
        uniform_name_to_location_[name] = location;
        return location;
    }

    return it->second;

}

void Shader::GenerateShaders(std::span<std::string_view> sources)
{
    GLenum types[ShaderIndex::kCount] = {GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER};

    std::size_t shader_index = 0;
    std::array<ShaderObject, ShaderIndex::kCount> shaders;

    for (const std::string_view& source : sources)
    {
        shaders[shader_index] = TryCompileShader(source.data(), static_cast<std::int32_t>(source.length()), types[shader_index]);
        shader_index++;
    }

    shader_program_ = glCreateProgram();

    for (std::size_t i = 0; i < shader_index; ++i)
    {
        shaders[i].AttachShaderToProgram(shader_program_);
    }

    glLinkProgram(shader_program_);

    std::int32_t linked_succesfully;

    glGetProgramiv(shader_program_, GL_LINK_STATUS, &linked_succesfully);

    if (linked_succesfully == GL_FALSE)
    {
        ThrowLinkingError(shader_program_);
    }
}

void Shader::GenerateShaders(std::span<const std::string> sources)
{
    ASSERT(sources.size() < ShaderIndex::kCount);
    std::array<std::string_view, ShaderIndex::kCount> srcs;
    std::uint32_t index = 0;

    for (const std::string& src : sources)
    {
        srcs[index++] = src;
    }

    GenerateShaders(std::span<std::string_view>{srcs.begin(), index});
}

GLint Shader::GetUniformLocation(const char* uniform_name) const
{
    auto it = uniform_name_to_location_.find(uniform_name);

    if (it == uniform_name_to_location_.end())
    {
        GLint location = glGetUniformLocation(shader_program_, uniform_name);
        uniform_name_to_location_[uniform_name] = location;

        return location;
    }

    return it->second;
}

void Shader::AddNewUniformInfo(std::vector<UniformInfo>& outUniformsInfo, int location) const
{
    const std::int32_t kMaxNameLength = 96;
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
    GLsizei name_len; // name length
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    glGetActiveUniform(shader_program_, static_cast<GLuint>(location), kMaxNameLength, &name_len, &size, &type, name);
    auto it = std::find_if(std::begin(kGLTypesToUniformTypes), std::end(kGLTypesToUniformTypes),
        [&](const GLTypeToUniformType& t) { return t.gl_uniform_type == type; });

    if (it != std::end(kGLTypesToUniformTypes))
    {
        outUniformsInfo.emplace_back(UniformInfo{it->uniform_type, name, location, static_cast<std::int32_t>(size)});
    }
}

