#include "vertex_buffer.h"

#include "renderer_api.h"
#include "opengl_vertex_buffer.h"
#include "error_macros.h"

std::shared_ptr<IVertexBuffer> IVertexBuffer::Create(const void* data, int32_t sizeBytes, bool bDynamic)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlVertexBuffer>(data, sizeBytes, bDynamic);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<IVertexBuffer> IVertexBuffer::CreateEmpty(int32_t maxSizeBytes)
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlVertexBuffer>(maxSizeBytes);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}
