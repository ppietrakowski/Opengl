#pragma once

#include "RenderCommand.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Transform.hpp"
#include "Sprite2D.hpp"
#include "CameraProjection.hpp"

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