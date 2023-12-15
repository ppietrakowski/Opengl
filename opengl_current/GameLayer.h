#pragma once

#include <typeindex>
#include <GL/glew.h>
#include "Event.h"

class IPlatform
{
public:
    IPlatform() { Instance = this; }
    virtual ~IPlatform() = default;

public:
    virtual glm::vec2 GetMousePosition() const = 0;
    virtual glm::vec2 GetLastMousePosition() const = 0;

    virtual void SetMouseVisible(bool mouseVisible) = 0;

    virtual bool IsKeyDown(std::int32_t key) const = 0;

    static IPlatform* GetInstance() { return Instance; }

private:
    static inline IPlatform* Instance = nullptr;
};

class Layer
{
public:
    Layer() :
        _game{ IPlatform::GetInstance() }
    {
    }

    virtual ~Layer() = default;

    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnRender(float deltaTime) = 0;
    virtual bool OnEvent(const Event& event) = 0;
    virtual void OnImguiFrame() = 0;

    virtual std::type_index GetTypeIndex() const
    {
        return typeid(Layer);
    }

    glm::vec2 GetMousePosition() const { return _game->GetMousePosition(); }
    glm::vec2 GetLastMousePosition() const { return _game->GetLastMousePosition(); }

    void SetMouseVisible(bool mouseVisible) { _game->SetMouseVisible(mouseVisible); }
    
    bool IsKeyDown(std::int32_t key) const { return _game->IsKeyDown(key); }

private:
    IPlatform* _game;
};
