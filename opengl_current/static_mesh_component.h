#pragma once

#include "static_mesh.h"

struct StaticMeshComponent
{
    std::string mesh_name;

    StaticMeshComponent() = default;
    StaticMeshComponent(const std::string& mesh);
};

