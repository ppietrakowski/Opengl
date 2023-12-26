#include "shader.h"
#include "logging.h"
#include "texture.h"

#include <string>
#include <fstream>
#include <vector>

#include <sstream>
#include <array>
#include <glm/gtc/type_ptr.hpp>

namespace {
    const char* ShaderTypeToString(GLenum type) {
        switch (type) {
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

    struct GLTypeToUniformType {
        GLenum gl_uniform_type;
        UniformType vertex_type;
    };

    /* Wraps shader object with RAII object */
    struct ShaderObject {
        GLuint shader_obj = 0;
        GLuint program = 0;

        ShaderObject() = default;
        ShaderObject(GLuint object) :
            shader_obj{ object } {}

        ShaderObject(ShaderObject&& temp_object) noexcept {
            *this = std::move(temp_object);
        }

        ShaderObject& operator=(ShaderObject&& temp_object) noexcept {
            shader_obj = temp_object.shader_obj;
            program = temp_object.program;
            temp_object.shader_obj = 0;
            temp_object.program = 0;
            return *this;
        }

        ~ShaderObject() {
            if (program != 0) {
                glDetachShader(program, shader_obj);
            } else if (shader_obj != 0) {
                glDeleteShader(shader_obj);
            }
        }

        void AttachShaderToProgram(GLuint program) {
            this->program = program;
            glAttachShader(program, shader_obj);
        }

        bool IsValid() const {
            return shader_obj != 0;
        }
    };

    void ThrowShaderCompilationError(GLuint shader, GLenum type) {
        std::vector<char> log;
        int32_t log_length;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        log.resize(log_length);
        glGetShaderInfoLog(shader, log_length, &log_length, log.data());

        std::string msg;

        msg = ShaderTypeToString(type);

        msg += " compilation failure: ";
        msg += log.data();

        ELOG_ERROR(LOG_RENDERER, "%s", msg.c_str());
        throw ShaderCompilationFailedException(msg.c_str());
    }

    ShaderObject TryCompileShader(const char* shader_source, int32_t length, GLenum type) {
        GLuint shader_object = glCreateShader(type);

        glShaderSource(shader_object, 1, &shader_source, &length);
        glCompileShader(shader_object);

        int32_t compiled_succesfully;

        glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compiled_succesfully);

        if (compiled_succesfully == GL_FALSE) {
            ThrowShaderCompilationError(shader_object, type);
        }

        return ShaderObject{ shader_object };
    }

    void ThrowLinkingError(GLuint program) {
        std::vector<char> log;

        int32_t log_length;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        log.resize(log_length + 1);
        log[log_length] = 0;

        glGetProgramInfoLog(program, log_length, &log_length, log.data());

        std::string msg;
        msg += "linking failure: ";
        msg += log.data();
        ELOG_ERROR(LOG_RENDERER, "%s", msg.c_str());

        glDeleteProgram(program);
        throw ShaderProgramLinkingFailedException(msg.c_str());
    }

    std::string LoadFileContent(const std::string& file_path) {
        std::ifstream file(file_path.c_str());
        file.exceptions(std::ios::failbit | std::ios::badbit);

        std::ostringstream content;

        content << file.rdbuf();

        return content.str();
    }
}

Shader::Shader(std::string_view vertex_shader_source, std::string_view fragment_shader_source) {
    std::array<std::string_view, 2> sources{ vertex_shader_source, fragment_shader_source };
    GenerateShaders(sources);
}

Shader::Shader(std::string_view vertex_shader_source, std::string_view fragment_shader_source,
    std::string_view geometry_shader_source) {
    std::array<std::string_view, 3> sources{ vertex_shader_source, fragment_shader_source, geometry_shader_source };
    GenerateShaders(sources);
}

Shader::Shader(std::string_view vertex_shader_source, std::string_view fragment_shader_source,
    std::string_view geometry_shader_source, std::string_view tesselation_control_shader_source,
    std::string_view tesselation_control_shader) {
    std::array<std::string_view, 5> sources{ vertex_shader_source, fragment_shader_source,
        geometry_shader_source, tesselation_control_shader_source, tesselation_control_shader };

    GenerateShaders(sources);
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path) {
    return LoadShader({ vertex_shader_path, fragment_shader_path });
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragment_shader_path,
    std::string_view geometry_shader_path) {
    return LoadShader({ vertexShaderPath, fragment_shader_path, geometry_shader_path });
}

std::shared_ptr<Shader> Shader::LoadShader(std::string_view vertexShaderPath, std::string_view fragment_shader_path,
    std::string_view geometry_shader_path, std::string_view tesselation_control_shader_path, std::string_view tesselation_evaluate_shader_path) {
    return LoadShader({ vertexShaderPath, fragment_shader_path,
        geometry_shader_path, tesselation_control_shader_path, tesselation_evaluate_shader_path });
}

Shader::Shader(Shader&& shader) noexcept {
    *this = std::move(shader);
}

Shader& Shader::operator=(Shader&& shader) noexcept {
    std::swap(shader.shader_program_, shader_program_);
    uniform_locations_cache_ = std::move(uniform_locations_cache_);
    return *this;
}

Shader::~Shader() {
    ELOG_VERBOSE(LOG_RENDERER, "Removing shader with ProgramID=%i", shader_program_);
    glDeleteProgram(shader_program_);
}

void Shader::Use() const {
    glUseProgram(shader_program_);
}

void Shader::StopUsing() const {
    glUseProgram(0);
}

void Shader::SetUniformInt(const char* name, int32_t  value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniformFloat(const char* name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniformVec2(const char* name, glm::vec2 value) {
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniformVec3(const char* name, const glm::vec3& value) {
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniformVec4(const char* name, const glm::vec4& value) {
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetUniformMat4(const char* name, const glm::mat4& value) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniformMat4Array(const char* name, std::span<const glm::mat4> values, uint32_t count) {
    glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, glm::value_ptr(values[0]));
}

void Shader::SetUniformMat3(const char* name, const glm::mat3& value) {
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

int32_t  Shader::GetUniformInt(const char* name) const {
    int32_t value = 0;
    glGetUniformiv(shader_program_, GetUniformLocation(name), &value);
    return value;
}

float Shader::GetUniformFloat(const char* name) const {
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value[0];
}

glm::vec2 Shader::GetUniformVec2(const char* name) const {
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec3 Shader::GetUniformVec3(const char* name) const {
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

glm::vec4 Shader::GetUniformVec4(const char* name) const {
    glm::vec4 value;
    glGetUniformfv(shader_program_, GetUniformLocation(name), glm::value_ptr(value));
    return value;
}

std::vector<UniformInfo> Shader::GetUniformInfos() const {
    GLint numUniforms;

    glGetProgramiv(shader_program_, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::vector<UniformInfo> uniforms_info;
    uniforms_info.reserve(numUniforms);

    for (GLint uniformLocation = 0; uniformLocation < numUniforms; uniformLocation++) {
        AddNewUniformInfo(uniforms_info, uniformLocation);
    }

    return uniforms_info;
}

void Shader::SetSamplerUniform(const char* uniform_name, const std::shared_ptr<Texture>& textures, uint32_t start_texture_unit) {

    ASSERT(start_texture_unit < kMinTextureUnits);
    glUniform1i(GetUniformLocation(uniform_name), static_cast<GLint>(start_texture_unit));
}

std::shared_ptr<Shader> Shader::LoadShader(const std::initializer_list<std::string_view>& paths) {
    std::array<std::string, ShaderIndex::kCount> sources;
    std::size_t index = 0;

    auto it = sources.begin();

    for (const std::string_view& path : paths) {
        sources[index++] = LoadFileContent(std::string{ path.begin(), path.end() });
        ++it;
    }

    std::array<std::string_view, ShaderIndex::kCount> sources_to_string_view;
    std::transform(sources.begin(), it, sources_to_string_view.begin(),
        [](const std::string& s) { return (std::string_view)s; });

    std::shared_ptr<Shader> shader = std::make_shared<Shader>();
    shader->GenerateShaders(std::span<std::string_view>{ sources_to_string_view.begin(), index });
    return shader;
}

void Shader::GenerateShaders(std::span<std::string_view> sources) {
    GLenum types[ShaderIndex::kCount] = { GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER };

    std::size_t shader_index = 0;
    std::array<ShaderObject, ShaderIndex::kCount> shaders;

    for (const std::string_view& source : sources) {
        shaders[shader_index] = TryCompileShader(source.data(), static_cast<int32_t>(source.length()), types[shader_index]);
        shader_index++;
    }

    shader_program_ = glCreateProgram();

    for (std::size_t i = 0; i < shader_index; ++i) {
        shaders[i].AttachShaderToProgram(shader_program_);
    }

    glLinkProgram(shader_program_);

    int32_t linked_succesfully;

    glGetProgramiv(shader_program_, GL_LINK_STATUS, &linked_succesfully);

    if (linked_succesfully == GL_FALSE) {
        ThrowLinkingError(shader_program_);
    }
}

int32_t Shader::GetUniformLocation(const char* uniform_name) const {
    auto it = uniform_locations_cache_.find(uniform_name);

    if (it == uniform_locations_cache_.end()) {
        int32_t location = glGetUniformLocation(shader_program_, uniform_name);
        uniform_locations_cache_[uniform_name] = location;

        return location;
    }

    return it->second;
}

void Shader::AddNewUniformInfo(std::vector<UniformInfo>& out_uniforms_info, GLint location) const {
    const uint32_t kMaxNameLength = 96;
    GLTypeToUniformType kGLTypesToUniformTypes[] =
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
    GLsizei name_length; // name length
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    glGetActiveUniform(shader_program_, static_cast<GLuint>(location), kMaxNameLength, &name_length, &size, &type, name);
    auto it = std::find_if(std::begin(kGLTypesToUniformTypes), std::end(kGLTypesToUniformTypes),
        [&](GLTypeToUniformType& t) { return t.gl_uniform_type == type; });

    if (it != std::end(kGLTypesToUniformTypes)) {
        out_uniforms_info.emplace_back(UniformInfo{ it->vertex_type, name, location, static_cast<uint32_t>(size) });
    }
}
