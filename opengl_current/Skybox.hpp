#pragma once

#include "Texture.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"

class Skybox
{
public:
    Skybox(std::shared_ptr<CubeMap> cubeMap, std::shared_ptr<Shader> shader);

    void Draw();

    static inline Skybox* s_Instance = nullptr;

    std::shared_ptr<CubeMap> GetCubeMap() const
    {
        return m_CubeMap;
    }

private:
    VertexArray m_VertexArray;
    std::shared_ptr<CubeMap> m_CubeMap;
    std::shared_ptr<Shader> m_Shader;
};

