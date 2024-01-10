#include "index_buffer.h"

#include "error_macros.h"
#include "renderer_api.h"
#include "opengl_index_buffer.h"

std::shared_ptr<IndexBuffer> IndexBuffer::Create(const std::uint32_t* data, std::int32_t numIndices, bool bDynamic)
{
    switch (RendererAPI::GetApi())
    {
    case RendererAPI::OpenGL:
        return std::make_shared<OpenGlIndexBuffer>(data, numIndices, bDynamic);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<IndexBuffer> IndexBuffer::CreateEmpty(std::int32_t totalnumIndices)
{
    switch (RendererAPI::GetApi())
    {
    case RendererAPI::OpenGL:
        return std::make_shared<OpenGlIndexBuffer>(totalnumIndices);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}
