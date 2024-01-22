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
    RgbaColor tint;
};

struct Transform2D {
    glm::vec2 translation{0, 0};
    float rotation{0};
    glm::vec2 scale{1, 1};
    glm::vec2 origin{0, 0};
    
    glm::mat4 GetTransformMatrix() const;
};

class Renderer2D {
public:
    static void Initialize();
    static void Quit();

    static void SetDrawShader(const std::shared_ptr<Shader>& shader);

    static void UpdateProjection(const CameraProjection& projection);

    static void DrawRect(const glm::vec2& mins, const glm::vec2& maxs, const Transform2D& transform, 
        const RgbaColor& color, int texture_id, glm::vec2 tilling = glm::vec2{1, 1});
    static void FlushDraw();

    static int BindTextureToDraw(const std::shared_ptr<Texture>& texture);
};