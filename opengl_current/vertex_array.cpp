#include "vertex_array.h"

#include "opengl_vertex_array.h"
#include "error_macros.h"
#include "renderer_api.h"

std::shared_ptr<IVertexArray> IVertexArray::Create()
{
    switch (IRendererAPI::GetApi())
    {
    case IRendererAPI::kOpenGL:
        return std::make_shared<OpenGlVertexArray>();
    }

    ERR_FAIL_MSG_V("Invalid RendererAPI type", nullptr);
}
