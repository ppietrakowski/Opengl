#pragma once

#include "Object.h"
#include "BoundingBox.h"

#include <unordered_map>
#include <memory>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform {
    glm::vec3 position{ 0, 0,0 };
    glm::quat rotation{ glm::vec3{0, 0,0} };
    glm::vec3 scale{ 1, 1, 1 };

    glm::mat4 GetTransform() const;
};

class Entity : public Object {
    DEFINE_CLASS_NOBASE();
public:
    Entity();

    // Inherited via Object
    Object* Clone() override;

    Entity* GetParent();
    const Entity* GetParent() const;

    void AddChild(Entity* child);
    void RemoveChild(const Entity& child);

    uint32_t GetNumChildren() const;
    void Destroy();

    bool IsPendingKill() const {
        return is_pending_kill_;
    }

    bool IsAncestorOf(const Entity& entity);

    bool IsSimulatingTicks() const {
        return is_simulating_ticks_;
    }

    glm::mat4 GetWorldTransformMatrix() const;
    glm::vec3 GetWorldPosition() const;

protected:
    BoundingBox bounds_;

protected:
    virtual void Tick(std::chrono::milliseconds delta_seconds);
    virtual void Render(std::chrono::milliseconds delta_seconds);

private:
    
    Entity* parent_{ nullptr };
    std::unordered_map<std::string, Entity*> children_;
    bool is_simulating_ticks_{ false };
    bool is_pending_kill_{ false };
    Transform transform_;
};

