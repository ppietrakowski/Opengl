#pragma once

#include "vertex_array.h"
#include "texture.h"
#include "material.h"

#include <glm/gtc/matrix_transform.hpp>

struct Line
{
    glm::vec3 StartPos{0.0f};
    glm::vec3 EndPos{0.0f};
};

struct SubmitCommandArgs
{
    std::shared_ptr<Material> UsedMaterial;
    int NumIndices{0};
    std::shared_ptr<VertexArray> TargetVertexArray;
    glm::mat4 Transform = glm::identity<glm::mat4>();

    const std::shared_ptr<Shader>& GetShader() const
    {
        assert(UsedMaterial);
        return UsedMaterial->GetShader();
    }

    void SetupShader() const
    {
        assert(UsedMaterial);
        UsedMaterial->SetupRenderState();
    }

    void ApplyMaterialUniforms() const
    {
        UsedMaterial->SetShaderUniforms();
    }
};