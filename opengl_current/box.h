#pragma once

#include "core.h"
#include <glm/glm.hpp>
#include <numeric>

struct Box
{
    glm::vec3 MinBounds;
    glm::vec3 MaxBounds;

    Box() :
        MinBounds{0.0f},
        MaxBounds{0.0f}
    {
    }

    Box(const glm::vec3& minBounds, const glm::vec3& maxBounds) :
        MinBounds(minBounds),
        MaxBounds(maxBounds)
    {
    }

    static Box FromOriginAndExtend(const glm::vec3& origin, const glm::vec3& extend)
    {
        return Box(origin - extend, origin + extend);
    }

    Box(const Box&) = default;
    Box& operator=(const Box&) = default;

    glm::vec3 GetOrigin() const
    {
        return (MaxBounds + MinBounds) / 2.0f;
    }

    glm::vec3 GetExtend() const
    {
        return (MaxBounds - MinBounds) / 2.0f;
    }

    Box TransformedBy(const glm::mat4& transform) const
    {
        glm::vec3 origin = transform * glm::vec4(GetOrigin(), 1.0f);
        return FromOriginAndExtend(origin, GetExtend());
    }

    bool IsIntersectingWithBox(const Box& box) const;
    bool IsIntersectingWithPoint(const glm::vec3& point) const;

    Box GetOverlap(const Box& other) const;
};


FORCE_INLINE bool Box::IsIntersectingWithBox(const Box& box) const
{
    return (MinBounds.x <= box.MaxBounds.x) && (MaxBounds.x >= box.MinBounds.x) &&
        (MinBounds.y <= box.MaxBounds.y) && (MaxBounds.y >= box.MinBounds.y) &&
        (MinBounds.z <= box.MaxBounds.z) && (MaxBounds.z >= box.MinBounds.z);
}

FORCE_INLINE bool Box::IsIntersectingWithPoint(const glm::vec3& point) const
{
    return (point.x >= MinBounds.x && point.x <= MaxBounds.x)
        && (point.y >= MinBounds.y && point.y <= MaxBounds.y)
        && (point.z >= MinBounds.z && point.z <= MaxBounds.z);
}

FORCE_INLINE Box Box::GetOverlap(const Box& other) const
{
    glm::vec3 mins(0.0f);
    glm::vec3 maxs(0.0f);

    mins.x = glm::max(MinBounds.x, other.MinBounds.x);
    maxs.x = glm::min(MaxBounds.x, other.MaxBounds.x);

    mins.y = glm::max(MinBounds.y, other.MinBounds.y);
    maxs.y = glm::min(MaxBounds.y, other.MaxBounds.y);

    mins.z = glm::max(MinBounds.z, other.MinBounds.z);
    maxs.z = glm::min(MaxBounds.z, other.MaxBounds.z);

    return Box(mins, maxs);
}
