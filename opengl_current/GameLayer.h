#pragma once

#include <typeindex>
#include <GL/glew.h>
#include "Event.h"

#include <chrono>

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


typedef std::chrono::duration<float, std::ratio<1, 1>> TimeSeconds;

inline std::chrono::milliseconds GetNow()
{
    using namespace std::chrono;
    auto duration = steady_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(duration);
}

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
