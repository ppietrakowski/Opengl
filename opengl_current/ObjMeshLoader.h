#pragma once

#include "MeshLoader.h"

class StaticObjMeshLoader : public IStaticMeshLoader
{
public:
    virtual bool Load(const std::string& path) override;

    virtual std::span<const StaticMeshVertex> GetVertices() const override;
    virtual std::span<const std::uint32_t> GetIndices() const override;
    virtual std::string_view GetModelName() const override;
    virtual std::string GetLastErrorMessage() const override;

private:
    std::vector<StaticMeshVertex> _vertices;
    std::vector<std::uint32_t> _indices;
    std::string _meshName;
    std::string _lastErrorMessage;

private:
    void AssignNewName(std::string& line);
};