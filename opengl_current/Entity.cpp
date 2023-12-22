#include "Entity.h"
#include "ErrorMacros.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/matrix_decompose.hpp>

IMPLEMENT_CLASS(Entity);

Entity::Entity() {}

Object* Entity::Clone() {
    return new Entity();
}

Entity* Entity::GetParent() {
    return parent_;
}

const Entity* Entity::GetParent() const {
    return parent_;
}

void Entity::AddChild(Entity* child) {

    ERR_FAIL_EXPECTED_TRUE(child->IsAncestorOf(*this));
    children_[child->GetName()] = child;
    child->parent_ = this;
}

void Entity::RemoveChild(const Entity& child) {
    
    children_.erase(child.GetName());
}

uint32_t Entity::GetNumChildren() const {
    return static_cast<uint32_t>(children_.size());
}

void Entity::Destroy() {
    if (parent_ != nullptr) {
        parent_->RemoveChild(*this);
    }
}

bool Entity::IsAncestorOf(const Entity& entity) {
    Entity* entity_parent = entity.parent_;
    while (entity_parent != nullptr) {
        if (entity_parent == this) {
            return true;
        }

        entity_parent = entity_parent->parent_;
    }
    return false;
}

glm::mat4 Entity::GetWorldTransformMatrix() const {

    glm::mat4 transform = transform_.GetTransform();
    Entity* current_parent = parent_;

    while (current_parent != nullptr) {
        transform = current_parent->transform_.GetTransform() * transform;
        current_parent = current_parent->parent_;
    }

    return transform;
}

glm::vec3 Entity::GetWorldPosition() const {
    return glm::vec3();
}

void Entity::Tick(std::chrono::milliseconds delta_seconds) {}

void Entity::Render(std::chrono::milliseconds delta_seconds) {
}

glm::mat4 Transform::GetTransform() const {
    return glm::translate(position) * glm::mat4_cast(rotation) * glm::scale(scale);
}
