#pragma once

#include "texture.h"
#include "vertex_array.h"
#include "shader.h"

class Skybox
{
public:
    Skybox(const std::shared_ptr<CubeMap>& cubeMap, const std::shared_ptr<Shader>& shader);

    void Draw();

    static inline Skybox* s_Instance = nullptr;

    std::shared_ptr<CubeMap> GetCubeMap() const
    {
        return m_CubeMap;
    }

private:
    std::unique_ptr<VertexArray> m_VertexArray;
    std::shared_ptr<CubeMap> m_CubeMap;
    std::shared_ptr<Shader> m_Shader;
};

