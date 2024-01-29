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

constexpr int32_t TakeNumIndicesFromVertexArray = 0;

class SubmitCommandArgs
{
public:
    SubmitCommandArgs() noexcept = default;
    SubmitCommandArgs(const Material& material, const VertexArray& vertexArray, int32_t numIndices = TakeNumIndicesFromVertexArray, const glm::mat4& transform = glm::identity<glm::mat4>()) noexcept;
    SubmitCommandArgs(const SubmitCommandArgs&) noexcept = default;
    SubmitCommandArgs& operator=(const SubmitCommandArgs&) noexcept = default;

    const std::shared_ptr<Shader>& GetShader() const
    {
        assert(IsValid());
        return m_Material->GetShader();
    }

    void SetupRenderState() const
    {
        assert(IsValid());
        m_Material->SetupRenderState();
    }

    void ApplyMaterialUniforms() const
    {
        assert(IsValid());
        m_Material->SetShaderUniforms();
    }

    const VertexArray& GetVertexArray() const
    {
        assert(IsValid());
        return *m_VertexArray;
    }

    bool IsValid() const
    {
        // All members are initialized using references, so only need to check one
        return m_Material != nullptr;
    }

    int32_t GetNumIndices() const
    {
        assert(IsValid());
        return m_NumIndices == TakeNumIndicesFromVertexArray ? m_VertexArray->GetNumIndices() : m_NumIndices;
    }

    const glm::mat4& GetTransform() const
    {
        return m_Transform;
    }

    uint32_t GetNumTexturesUsed() const
    {
        return m_Material->GetNumTextures();
    }

private:
    const Material* m_Material{nullptr};
    int32_t m_NumIndices{TakeNumIndicesFromVertexArray};
    const VertexArray* m_VertexArray{nullptr};
    glm::mat4 m_Transform = glm::identity<glm::mat4>();
};

FORCE_INLINE SubmitCommandArgs::SubmitCommandArgs(const Material& material, const VertexArray& vertexArray, int32_t numIndices, const glm::mat4& transform) noexcept :
    m_Material(&material),
    m_NumIndices(numIndices),
    m_VertexArray(&vertexArray),
    m_Transform(transform)
{
    assert(m_NumIndices <= m_VertexArray->GetNumIndices());
}