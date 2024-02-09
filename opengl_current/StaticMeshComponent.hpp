#pragma once

#include "StaticMesh.hpp"

struct StaticMeshComponent
{
    std::string MeshName;

    StaticMeshComponent() = default;
    StaticMeshComponent(const std::string& mesh);
};

