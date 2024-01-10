#include "shader.h"

#include "opengl_shader.h"
#include "renderer_api.h"
#include "error_macros.h"

#include <sstream>
#include <array>
#include <fstream>
#include <algorithm>

std::shared_ptr<Shader> Shader::CreateFromSource(std::span<const std::string> sources)
{
    std::shared_ptr<Shader> shader = nullptr;

    switch (RendererAPI::GetApi())
    {
    case RendererAPI::OpenGL:
        shader = std::make_shared<OpenGlShader>();
        break;
    default:
        ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
    }

    shader->GenerateShaders(sources);
    return shader;
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

std::shared_ptr<Shader> ShaderSourceBuilder::Build()
{
    std::uint32_t index = GetLastShaderIndex();
    return Shader::CreateFromSource(std::span<const std::string>{m_ShaderSources.begin(), index});
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
