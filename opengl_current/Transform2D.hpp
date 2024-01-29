#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform2D
{
    glm::vec2 Position{0, 0};
    float Rotation{0};
    glm::vec2 Size{1, 1};
    glm::vec2 Origin{0, 0};

    glm::mat4 GetTransformMatrix() const;
};


inline glm::mat4 Transform2D::GetTransformMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(Position, 0.0f));

    model = glm::translate(model, glm::vec3(Origin, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-Origin, 0.0f));

    model = glm::scale(model, glm::vec3(Size, 1.0f));

    return model;
}