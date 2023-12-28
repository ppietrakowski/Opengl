#include "index_buffer.h"

#include "error_macros.h"
#include "renderer_api.h"
#include "opengl_index_buffer.h"

std::shared_ptr<IIndexBuffer> IIndexBuffer::Create(const uint32_t* data, uint32_t numIndices, bool bDynamic) {
    switch (IRendererAPI::GetApi()) {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlIndexBuffer>(data, numIndices, bDynamic);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}

std::shared_ptr<IIndexBuffer> IIndexBuffer::CreateEmpty(uint32_t totalNumIndices) {
    switch (IRendererAPI::GetApi()) {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlIndexBuffer>(totalNumIndices);
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}
