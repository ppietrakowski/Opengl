#pragma once

#include "core.h"
#include <glm/glm.hpp>
#include <numeric>

struct Box {
    glm::vec3 min_bounds;
    glm::vec3 max_bounds;

    Box() :
        min_bounds{0.0f},
        max_bounds{0.0f} {
    }

    Box(const glm::vec3& min_bounds, const glm::vec3& max_bounds) :
        min_bounds(min_bounds),
        max_bounds(max_bounds) {
    }

    static Box FromOriginAndExtend(const glm::vec3& origin, const glm::vec3& extend) {
        return Box(origin - extend, origin + extend);
    }

    Box(const Box&) = default;
    Box& operator=(const Box&) = default;

    glm::vec3 GetOrigin() const {
        return (max_bounds + min_bounds) / 2.0f;
    }

    glm::vec3 GetExtend() const {
        return (max_bounds - min_bounds) / 2.0f;
    }

    Box TransformedBy(const glm::mat4& transform) const {
        glm::vec3 origin = transform * glm::vec4(GetOrigin(), 1.0f);
        return FromOriginAndExtend(origin, GetExtend());
    }

    bool IsIntersectingWithBox(const Box& box) const;
    bool IsIntersectingWithPoint(const glm::vec3& point) const;

    Box GetOverlap(const Box& other) const;
};


FORCE_INLINE bool Box::IsIntersectingWithBox(const Box& box) const {
    return (min_bounds.x <= box.max_bounds.x) && (max_bounds.x >= box.min_bounds.x) &&
        (min_bounds.y <= box.max_bounds.y) && (max_bounds.y >= box.min_bounds.y) &&
        (min_bounds.z <= box.max_bounds.z) && (max_bounds.z >= box.min_bounds.z);
}

FORCE_INLINE bool Box::IsIntersectingWithPoint(const glm::vec3& point) const {
    return (point.x >= min_bounds.x && point.x <= max_bounds.x)
        && (point.y >= min_bounds.y && point.y <= max_bounds.y)
        && (point.z >= min_bounds.z && point.z <= max_bounds.z);
}

FORCE_INLINE Box Box::GetOverlap(const Box& other) const {
    glm::vec3 mins(0.0f);
    glm::vec3 maxs(0.0f);

    mins.x = glm::max(min_bounds.x, other.min_bounds.x);
    maxs.x = glm::min(max_bounds.x, other.max_bounds.x);

    mins.y = glm::max(min_bounds.y, other.min_bounds.y);
    maxs.y = glm::min(max_bounds.y, other.max_bounds.y);

    mins.z = glm::max(min_bounds.z, other.min_bounds.z);
    maxs.z = glm::min(max_bounds.z, other.max_bounds.z);

    return Box(mins, maxs);
}
