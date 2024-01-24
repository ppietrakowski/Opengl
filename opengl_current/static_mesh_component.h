#pragma once

#include "static_mesh.h"

struct StaticMeshComponent
{
    std::string MeshName;

    StaticMeshComponent() = default;
    StaticMeshComponent(const std::string& mesh);
};

