#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>


#include "ErrorMacros.h"
#include "RenderCommand.h"

#include <array>

glm::mat4 Renderer::View{ 1.0f };
glm::mat4 Renderer::Projection{ 1.0f };
glm::mat4 Renderer::ProjectionView{ 1.0f };
glm::vec3 Renderer::CameraLocation{ 0.0f, 0.0f, 0.0f };
std::shared_ptr<Texture2D> Renderer::DefaultTexture;

static unsigned int BoxIndices[] =
{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

#define MAX_DEBUG_NUM_BOX 100
#define NUM_BOX_VERTICES 8

struct BoxBatchData
{
    glm::vec3 Vertices[MAX_DEBUG_NUM_BOX * NUM_BOX_VERTICES];
    unsigned int NumBoxes{ MAX_DEBUG_NUM_BOX };
    unsigned int CurrentNumBox{ 0 };
    std::unique_ptr<VertexArray> VertexArray;
};

static BoxBatchData *BoxBatch = nullptr;

void Renderer::Quit()
{
    delete BoxBatch;
    DefaultTexture.reset();
}

struct RgbColor
{
    std::uint8_t Red;
    std::uint8_t Green;
    std::uint8_t Blue;
};

void Renderer::Initialize()
{
    RgbColor colors[4 * 4] =
    {
        {0, 0, 0}, {0, 0, 0},         {255, 0, 255}, {255, 0, 255},
        {0, 0, 0}, {0, 0, 0},         {255, 0, 255}, {255, 0, 255},
        {255, 0, 255}, {255, 0, 255}, {0, 0, 0},
        {255, 0, 255}, {255, 0, 255}, {0, 0, 0},
    };

    DefaultTexture = std::make_shared<Texture2D>(colors, 4, 4, ETextureFormat::Rgb);
    glEnable(GL_DEPTH_TEST);
    BoxBatch = new BoxBatchData();
    BoxBatch->VertexArray = std::make_unique<VertexArray>();
    VertexAttribute attributes[] = { {3, EPrimitiveVertexType::Float} };
    VertexBuffer buffer(NUM_BOX_VERTICES * MAX_DEBUG_NUM_BOX * sizeof(glm::vec3));

    BoxBatch->VertexArray->AddBuffer(std::move(buffer), attributes);

    IndexBuffer indexBuffer(sizeof(BoxIndices) / sizeof(BoxIndices[0]) * MAX_DEBUG_NUM_BOX);

    std::vector<unsigned int> indices;
    unsigned int currentIndiceOffset = 0;
    unsigned int maxNumIndices = static_cast<unsigned int>(indexBuffer.GetNumIndices() / MAX_DEBUG_NUM_BOX);

    indices.reserve(MAX_DEBUG_NUM_BOX *maxNumIndices);

    unsigned int offsetToNextFreeIndex = static_cast<int32_t>(NUM_BOX_VERTICES);

    for (uint32_t currentMeshNo = 0; currentMeshNo < MAX_DEBUG_NUM_BOX; currentMeshNo++)
    {
        for (size_t i = 0; i < maxNumIndices; i++)
        {
            size_t vertexIndex = static_cast<size_t>(BoxIndices[i]) + currentIndiceOffset;
            indices.push_back(static_cast<uint32_t>(vertexIndex));
        }

        currentIndiceOffset += offsetToNextFreeIndex;
    }
    indexBuffer.UpdateIndices(indices.data(), static_cast<unsigned int>(indices.size()));

    BoxBatch->VertexArray->SetIndexBuffer(std::move(indexBuffer));

    RenderCommand::SetCullFace(true);
}

void Renderer::UpdateProjection(float width, float height, float fov, float zNear, float zFar)
{
    float aspectRatio = width / height;
    Projection = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
}

void Renderer::BeginScene(const glm::mat4& view, glm::vec3 cameraPosition)
{
    View = view;
    ProjectionView = Projection * view;
    CameraLocation = cameraPosition;
}

void Renderer::EndScene()
{
    BoxBatch->CurrentNumBox = 0;
}

void Renderer::Submit(const Material& material, const VertexArray& vertexArray, const glm::mat4& transform, ERenderPrimitive renderPrimitive)
{
    Shader& shader = material.GetShader();
    shader.Use();
    material.SetupRenderState();
    material.SetShaderUniforms();

    shader.SetUniformMat4("u_ProjectionView", ProjectionView);
    shader.SetUniformMat4("u_Transform", transform);
    shader.SetUniformVec3("u_CameraLocation", CameraLocation);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_NormalTransform", normalMatrix);

    RenderCommand::DrawIndexed(vertexArray, vertexArray.GetNumIndices(), renderPrimitive);
}

void Renderer::Submit(Shader& shader,
    const VertexArray& vertexArray,
    const glm::mat4& transform,
    ERenderPrimitive renderPrimitive)
{
    Submit(shader, vertexArray.GetNumIndices(), vertexArray, transform, renderPrimitive);
}

void Renderer::Submit(Shader& shader, unsigned int numIndices, const VertexArray& vertexArray, const glm::mat4& transform, ERenderPrimitive renderPrimitive)
{
    MAYBE(numIndices <= vertexArray.GetNumIndices());

    shader.Use();
    shader.SetUniformMat4("u_ProjectionView", ProjectionView);
    shader.SetUniformMat4("u_Transform", transform);
    shader.SetUniformVec3("u_CameraLocation", CameraLocation);

    glm::mat3 normalMatrix = glm::inverseTranspose(transform);
    shader.SetUniformMat3("u_NormalTransform", normalMatrix);

    RenderCommand::DrawIndexed(vertexArray, numIndices, renderPrimitive);
}

void Renderer::AddDebugBox(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform)
{
    std::array<glm::vec3, 8> vertices = {
        glm::vec3{boxmin[0], boxmin[1], boxmin[2]},
        glm::vec3{boxmax[0], boxmin[1], boxmin[2]},
        glm::vec3{boxmax[0], boxmax[1], boxmin[2]},
        glm::vec3{boxmin[0], boxmax[1], boxmin[2]},

        glm::vec3{boxmin[0], boxmin[1], boxmax[2]},
        glm::vec3{boxmax[0], boxmin[1], boxmax[2]},
        glm::vec3{boxmax[0], boxmax[1], boxmax[2]},
        glm::vec3{boxmin[0], boxmax[1], boxmax[2]}
    };

    if (BoxBatch->CurrentNumBox >= BoxBatch->NumBoxes)
    {
        return;
    }

    size_t maxNumVertices = vertices.size();

    for (size_t i = 0; i < maxNumVertices; ++i)
    {
        glm::vec3& vertex = BoxBatch->Vertices[i + BoxBatch->CurrentNumBox * NUM_BOX_VERTICES];
        vertex = transform * glm::vec4{ vertices[i], 1.0f };
    }

    BoxBatch->CurrentNumBox++;
}

void Renderer::FlushDrawDebug(Shader& shader)
{
    BoxBatch->VertexArray->GetVertexBufferAt(0).UpdateVertices(BoxBatch->Vertices, sizeof(glm::vec3) * BoxBatch->CurrentNumBox * NUM_BOX_VERTICES);
    Submit(shader, BoxBatch->CurrentNumBox * (sizeof(BoxIndices) / sizeof(BoxIndices[0])), *BoxBatch->VertexArray, glm::mat4{ 1.0 }, ERenderPrimitive::Lines);
}
