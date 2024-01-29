#pragma once

#include "render_command.h"
#include "shader.h"
#include "vertex_array.h"
#include "texture.h"
#include "material.h"
#include "transform.h"
#include "Sprite2D.h"
#include "camera_projection.h"

class Renderer2D
{
    friend class Game;

private:
    static void Initialize();
    static void Quit();

public:
    static void SetDrawShader(const std::shared_ptr<Shader>& shader);

    static void UpdateProjection(const CameraProjection& projection);

    static void DrawSprite(const Sprite2D& sprite);
    static void FlushDraw();

    static std::int32_t BindTextureToDraw(const std::shared_ptr<Texture>& texture);
};