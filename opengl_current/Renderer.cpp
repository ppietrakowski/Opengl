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

// Predefined box indices (base for offsets for IndexBuffer)
static std::uint32_t BoxIndices[] =
{
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
};

constexpr std::size_t MaxDebugNumBox = 100;
constexpr std::size_t NumBoxVertices = 8;

struct BoxBatchData
{
    glm::vec3 Vertices[MaxDebugNumBox * NumBoxVertices];
    std::uint32_t NumBoxes{ MaxDebugNumBox };
    std::uint32_t CurrentNumBox{ 0 };
    VertexArray DebugVertexArray;

    BoxBatchData()
    {
        // initialize box batching
        VertexAttribute attributes[] = { {3, PrimitiveVertexType::Float} };
        VertexBuffer buffer();

        DebugVertexArray.AddDynamicBuffer(static_cast<std::uint32_t>(NumBoxVertices * MaxDebugNumBox * sizeof(glm::vec3)), attributes);

        // prebatch indices
        std::vector<std::uint32_t> indices;
        std::uint32_t currentIndicesStartOffset = 0;
        std::uint32_t maxNumIndices = static_cast<std::uint32_t>(ARRAY_NUM_ELEMENTS(BoxIndices));

        indices.reserve(MaxDebugNumBox * maxNumIndices);

        // current starting index of mesh index
        std::uint32_t offsetToNextFreeIndex = NumBoxVertices;

        for (std::uint32_t currentMeshNo = 0; currentMeshNo < MaxDebugNumBox; currentMeshNo++)
        {
            for (std::uint32_t i = 0; i < maxNumIndices; i++)
            {
                std::uint32_t vertexIndex = BoxIndices[i] + currentIndicesStartOffset;
                indices.emplace_back(vertexIndex);
            }

            currentIndicesStartOffset += offsetToNextFreeIndex;
        }


        IndexBuffer indexBuffer(indices.data(), static_cast<std::uint32_t>(indices.size()));
        DebugVertexArray.SetIndexBuffer(std::move(indexBuffer));
    }

    void UpdateBuffers()
    {
        // only the vertex buffer should be updated, because indices are prebatched 
        VertexBuffer& vertexBuffer = DebugVertexArray.GetVertexBufferAt(0);
        vertexBuffer.UpdateVertices(Vertices, sizeof(glm::vec3) * CurrentNumBox * NumBoxVertices);
    }

    void FlushDraw(Shader& shader)
    {
        Renderer::Submit(shader, CurrentNumBox * ARRAY_NUM_ELEMENTS(BoxIndices), DebugVertexArray, glm::mat4{1.0}, RenderPrimitive::Lines);
        CurrentNumBox = 0;
    }

    void AddInstance(glm::vec3 boxmin, glm::vec3 boxmax, const glm::mat4& transform)
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

        if (CurrentNumBox >= NumBoxes)
        {
            return;
        }

        std::uint32_t maxNumVertices = static_cast<std::uint32_t>(vertices.size());

        for (std::uint32_t i = 0; i < maxNumVertices; ++i)
        {
            glm::vec3& vertex = Vertices[i + CurrentNumBox * NumBoxVertices];
            vertex = transform * glm::vec4{ vertices[i], 1.0f };
        }

        CurrentNumBox++;
    }
};

static BoxBatchData* BoxBatch = nullptr;

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

    RgbColor() = default;
    RgbColor(const RgbColor&) = default;
    RgbColor& operator=(const RgbColor&) = default;

    constexpr RgbColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue) :
        Red{ red },
        Green{green},
        Blue{ blue }
    {
    }
};

static constexpr RgbColor Black{0, 0, 0};
static constexpr RgbColor Magenta{255, 0, 255};

void Renderer::Initialize()
{
    // array of checkerboard with black and magenta
    RgbColor colors[4 * 4] =
    {
        Black, Black, Magenta, Magenta,
        Black, Black, Magenta, Magenta,
        Magenta, Magenta, Black, Black,
        Magenta, Magenta, Black, Black
    };

    DefaultTexture = std::make_shared<Texture2D>(colors, 4, 4, TextureFormat::Rgb);

    glEnable(GL_DEPTH_TEST);

    BoxBatch = new BoxBatchData();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
}

void Renderer::Submit(const Material& material, const VertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
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

void Renderer::SubmitSkeleton(const Material& material, std::span<const glm::mat4> transforms, std::uint32_t count, const VertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
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
    shader.SetUniformMat4Array("bone_transforms", transforms, count);
    RenderCommand::DrawIndexed(vertexArray, vertexArray.GetNumIndices(), renderPrimitive);
}

void Renderer::Submit(Shader& shader,
    const VertexArray& vertexArray,
    const glm::mat4& transform,
    RenderPrimitive renderPrimitive)
{
    Submit(shader, vertexArray.GetNumIndices(), vertexArray, transform, renderPrimitive);
}

void Renderer::Submit(Shader& shader, std::uint32_t numIndices, const VertexArray& vertexArray, const glm::mat4& transform, RenderPrimitive renderPrimitive)
{
    ASSERT(numIndices <= vertexArray.GetNumIndices());

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
    BoxBatch->AddInstance(boxmin, boxmax, transform);
}

void Renderer::FlushDrawDebug(Shader& shader)
{
    BoxBatch->UpdateBuffers();
    BoxBatch->FlushDraw(shader);
}
