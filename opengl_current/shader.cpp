#include "shader.h"

#include "opengl_shader.h"
#include "renderer_api.h"
#include "error_macros.h"

#include <sstream>
#include <array>
#include <fstream>
#include <algorithm>

std::shared_ptr<Shader> Shader::CreateFromSource(std::span<const std::string> sources) {
    std::shared_ptr<Shader> shader = nullptr;

    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        shader = std::make_shared<OpenGlShader>();
        break;
    default:
        ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
    }

    shader->GenerateShaders(sources);
    return shader;
}

void Shader::GenerateShaders(std::span<const std::string> sources) {
    ASSERT(sources.size() < ShaderIndex::kCount);
    std::array<std::string_view, ShaderIndex::kCount> srcs;
    std::uint32_t index = 0;

    for (const std::string& src : sources) {
        srcs[index++] = src;
    }

    GenerateShaders(std::span<std::string_view>{srcs.begin(), index});
}

std::uint32_t ShaderSourceBuilder::GetLastShaderIndex() const {
    for (auto it = shader_sources_.rbegin(); it != shader_sources_.rend(); ++it) {
        if (it->empty()) {
            return static_cast<std::uint32_t>(std::distance(shader_sources_.rbegin(), it));
        }
    }

    return ShaderIndex::kCount;
}

std::shared_ptr<Shader> ShaderSourceBuilder::Build() {
    std::uint32_t index = GetLastShaderIndex();
    return Shader::CreateFromSource(std::span<const std::string>{shader_sources_.begin(), index});
}

ShaderSourceBuilder& ShaderSourceBuilder::SetVertexShaderSource(const std::string& source) {
    shader_sources_[ShaderIndex::kVertex] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadVertexShaderSource(const std::filesystem::path& file_path) {
    shader_sources_[ShaderIndex::kVertex] = LoadFileContent(file_path);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetFragmentShaderSource(const std::string& source) {
    shader_sources_[ShaderIndex::kFragment] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadFragmentShaderSource(const std::filesystem::path& file_path) {
    shader_sources_[ShaderIndex::kFragment] = LoadFileContent(file_path);
    return *this;
}


ShaderSourceBuilder& ShaderSourceBuilder::SetGeometryShaderSource(const std::string& source) {
    shader_sources_[ShaderIndex::kGeometry] = source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadGeometryShaderSource(const std::filesystem::path& file_path) {
    shader_sources_[ShaderIndex::kGeometry] = LoadFileContent(file_path);
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::SetTesselationShaderSource(const std::string& control_shader_source,
    const std::string& evaluate_shader_source) {
    shader_sources_[ShaderIndex::kTesselationControlShader] = control_shader_source;
    shader_sources_[ShaderIndex::kTesselationEvaluateShader] = evaluate_shader_source;
    return *this;
}

ShaderSourceBuilder& ShaderSourceBuilder::LoadGeometryShaderSource(const std::filesystem::path& control_shader_path,
    const std::filesystem::path& evaluate_shader_source) {
    shader_sources_[ShaderIndex::kTesselationControlShader] = LoadFileContent(control_shader_path);
    shader_sources_[ShaderIndex::kTesselationEvaluateShader] = LoadFileContent(evaluate_shader_source);
    return *this;
}
