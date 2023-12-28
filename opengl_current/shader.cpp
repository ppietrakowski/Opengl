#include "shader.h"

#include "opengl_shader.h"
#include "renderer_api.h"
#include "error_macros.h"

#include <sstream>
#include <array>
#include <fstream>
#include <algorithm>

static std::string LoadFileContent(const std::string& file_path)
{
    std::ifstream file(file_path.c_str());
    file.exceptions(std::ios::failbit | std::ios::badbit);

    std::ostringstream content;

    content << file.rdbuf();

    return content.str();
}

std::shared_ptr<IShader> IShader::CreateFromSource(std::string_view vertex_shader_source, std::string_view fragment_shader_source)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlShader>(vertex_shader_source, fragment_shader_source);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<IShader> IShader::CreateFromSource(std::string_view vertex_shader_source, std::string_view fragment_shader_source,
    std::string_view geometry_shader_source)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlShader>(vertex_shader_source, fragment_shader_source, geometry_shader_source);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<IShader> IShader::CreateFromSource(std::string_view vertex_shader_source, std::string_view fragment_shader_source,
    std::string_view geometry_shader_source, std::string_view tesselation_control_shader, std::string_view tesselation_evaluate_shader)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlShader>(vertex_shader_source, fragment_shader_source, geometry_shader_source,
            tesselation_control_shader, tesselation_evaluate_shader);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<IShader> IShader::LoadShader(std::string_view vertex_shader_path, std::string_view fragment_shader_path)
{
    return LoadShader({vertex_shader_path, fragment_shader_path});
}

std::shared_ptr<IShader> IShader::LoadShader(std::string_view vertexShaderPath, std::string_view fragment_shader_path,
    std::string_view geometry_shader_path)
{
    return LoadShader({vertexShaderPath, fragment_shader_path, geometry_shader_path});
}

std::shared_ptr<IShader> IShader::LoadShader(std::string_view vertexShaderPath, std::string_view fragment_shader_path,
    std::string_view geometry_shader_path, std::string_view tesselation_control_shader_path, std::string_view tesselation_evaluate_shader_path)
{
    return LoadShader({vertexShaderPath, fragment_shader_path,
        geometry_shader_path, tesselation_control_shader_path, tesselation_evaluate_shader_path});
}


std::shared_ptr<IShader> IShader::LoadShader(const std::initializer_list<std::string_view>& paths)
{
    std::array<std::string, ShaderIndex::kCount> sources;
    std::size_t index = 0;

    auto it = sources.begin();

    for (const std::string_view& path : paths)
    {
        sources[index++] = LoadFileContent(std::string{path.begin(), path.end()});
        ++it;
    }

    std::array<std::string_view, ShaderIndex::kCount> sources_to_string_view;
    std::transform(sources.begin(), it, sources_to_string_view.begin(),
        [](const std::string& s) { return static_cast<std::string_view>(s); });

    std::shared_ptr<IShader> shader = std::make_shared<OpenGlShader>();
    shader->GenerateShaders(std::span<std::string_view>{ sources_to_string_view.begin(), index });
    return shader;
}