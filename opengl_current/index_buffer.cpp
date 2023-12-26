#include "index_buffer.h"

#include "error_macros.h"
#include "renderer_api.h"
#include "opengl_index_buffer.h"

std::shared_ptr<IndexBuffer> IndexBuffer::Create(const uint32_t* data, uint32_t num_indices, bool dynamic) {
    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        return std::make_shared<OpenGlIndexBuffer>(data, num_indices, dynamic);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<IndexBuffer> IndexBuffer::CreateEmpty(uint32_t total_num_indices) {
    switch (RendererAPI::GetApi()) {
    case RendererAPI::kOpenGL:
        return std::make_shared<OpenGlIndexBuffer>(total_num_indices);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}
