#include "vertex_buffer.h"

#include "renderer_api.h"
#include "opengl_vertex_buffer.h"
#include "error_macros.h"

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void* data, std::int32_t size_bytes, bool dynamic) {
    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        return std::make_shared<OpenGlVertexBuffer>(data, size_bytes, dynamic);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<VertexBuffer> VertexBuffer::CreateEmpty(std::int32_t max_size_bytes) {
    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        return std::make_shared<OpenGlVertexBuffer>(max_size_bytes);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}
