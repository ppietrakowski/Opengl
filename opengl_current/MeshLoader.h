#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>
#include <span>

#include <string>
#include <memory>

#include <cstdint>
#include <filesystem>


struct StaticMeshVertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TextureCoords;

    static inline constexpr VertexAttribute DataFormat[3] = { {3, EPrimitiveVertexType::Float}, {3, EPrimitiveVertexType::Float}, {2, EPrimitiveVertexType::Float} };
};

class IStaticMeshLoader
{
public:
    virtual ~IStaticMeshLoader() = default;

    /*
    * Loads mesh from specified path
    * Should return true, if no error occured during loading
    */
    virtual bool Load(const std::string& path) = 0;

    virtual std::span<const StaticMeshVertex> GetVertices() const = 0;
    virtual std::span<const unsigned int> GetIndices() const = 0;
    virtual std::string_view GetModelName() const = 0;

    virtual std::string GetLastErrorMessage() const = 0;
};

class StaticMeshImporter
{
public:
    StaticMeshImporter(std::filesystem::path path);

    std::span<const StaticMeshVertex> GetVertices() const;
    std::span<const unsigned int> GetIndices() const;
    std::string_view GetModelName() const;

    std::string GetLastErrorMessage() const;

    std::runtime_error GetError() const;

    bool HasErrorOccured() const;

    unsigned int GetNumIndices() const;
    unsigned int GetNumVertices() const;

private:
    std::unique_ptr<IStaticMeshLoader> _staticMeshLoader;
    std::string _errorMessage;
    bool _errorOccured : 1;
};