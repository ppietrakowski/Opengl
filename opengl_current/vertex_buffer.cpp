#include "vertex_buffer.h"

#include "renderer_api.h"
#include "opengl_vertex_buffer.h"
#include "error_macros.h"

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void* data, std::int32_t sizeBytes, bool bDynamic)
{
    switch (RendererAPI::GetApi())
    {
    case RendererAPI::OpenGL:
        return std::make_shared<OpenGlVertexBuffer>(data, sizeBytes, bDynamic);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<VertexBuffer> VertexBuffer::CreateEmpty(std::int32_t maxSizeBytes)
{
    switch (RendererAPI::GetApi())
    {
    case RendererAPI::OpenGL:
        return std::make_shared<OpenGlVertexBuffer>(maxSizeBytes);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}
