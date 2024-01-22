#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"
#include "batching.h"
#include "camera_projection.h"

struct SpriteVertex {
    glm::vec2 position;
    glm::vec2 texture_coords;
    int texture_index;
    RgbaColor color;
};

template<>
struct BatchVertexCreator<SpriteVertex> {
    static SpriteVertex CreateInstanceFrom(const SpriteVertex& vertex, const glm::mat4&, const Transform2D& transform_2d) {
        SpriteVertex v{vertex};
        const glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(transform_2d.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3{transform_2d.origin, 0.0f});
        rotationMatrix = rotationMatrix * rotationZ;
        rotationMatrix = glm::translate(rotationMatrix, glm::vec3{-transform_2d.origin, 0.0f});

        glm::mat4 transform_matrix = glm::translate(glm::mat4(1.0f), glm::vec3{transform_2d.translation + transform_2d.origin, 0.0f}) *
            rotationMatrix * glm::scale(glm::mat4(1.0f), glm::vec3{transform_2d.scale, 0.0f});

        v.position = transform_matrix * glm::vec4{v.position, 0, 1};
        return v;
    }
};

struct Transform2D {
    glm::vec2 translation;
    float rotation;
    glm::vec2 scale;
    glm::vec2 origin;
};

class Renderer2D {
public:
    static void Initialize();
    static void Quit();

    static void SetDrawShader(const std::shared_ptr<Shader>& shader);

    static void UpdateProjection(const CameraProjection& projection);

    static void DrawRect(const glm::vec2& mins, const glm::vec2& maxs, const Transform2D& transform, const RgbaColor& color, int texture_id);
    static void FlushDraw();

    static int BindTextureToDraw(const std::shared_ptr<Texture>& texture);
};