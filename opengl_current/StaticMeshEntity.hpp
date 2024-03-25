#pragma once

#include "Entity.hpp"

class StaticMeshEntity : public BaseEntity
{
public:
    DEFINE_CLASS_BASE(StaticMeshEntity, BaseEntity);
    std::string StaticMeshPath;
};

