#pragma once

#include "texture.h"
#include "vertex_array.h"
#include "shader.h"

class Skybox {
public:
    Skybox(const std::shared_ptr<CubeMap>& cube_map, const std::shared_ptr<Shader>& shader);

    void Draw();

private:
    std::shared_ptr<VertexArray> vertex_array_;
    std::shared_ptr<CubeMap> cube_map_;
    std::shared_ptr<Shader> shader_;
};

