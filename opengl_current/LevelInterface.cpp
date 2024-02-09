#include "LevelInterface.hpp"

#include <glm/gtc/matrix_transform.hpp>

glm::vec3 LevelInterface::ProjectWorldToScreen(glm::vec3 world)
{
    glm::mat4 model = s_View;
    return glm::project(world, model, s_Projection, glm::vec4(s_Viewport.StartPosition, s_Viewport.Size));
}

glm::vec3 LevelInterface::ProjectScreenToWorld(glm::vec3 screen)
{
    glm::mat4 model = s_View;
    return glm::unProject(screen, model, s_Projection, glm::vec4(s_Viewport.StartPosition, s_Viewport.Size));
}

void LevelInterface::BeginScene(const glm::mat4& projection, const glm::mat4& view, const Viewport& viewport)
{
    s_Projection = projection;
    s_View = view;
    s_Viewport = viewport;
}
