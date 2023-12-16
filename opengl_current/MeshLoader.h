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
    glm::vec3 Position{ 0, 0,0 };
    glm::vec3 Normal{ 0, 0, 0 };
    glm::vec2 TextureCoords{ 0, 0 };


    static inline constexpr VertexAttribute DataFormat[3] = { {3, PrimitiveVertexType::Float}, {3, PrimitiveVertexType::Float}, {2, PrimitiveVertexType::Float} };

    StaticMeshVertex() = default;
    StaticMeshVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textureCoords) :
        Position{ position },
        Normal{ normal },
        TextureCoords{ textureCoords }
    {
    }

    StaticMeshVertex(const StaticMeshVertex&) = default;
    StaticMeshVertex& operator=(const StaticMeshVertex&) = default;
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
    virtual std::span<const std::uint32_t> GetIndices() const = 0;
    virtual std::string_view GetModelName() const = 0;

    virtual std::string GetLastErrorMessage() const = 0;
};

class StaticMeshImporter
{
public:
    StaticMeshImporter(std::filesystem::path path);

    std::span<const StaticMeshVertex> GetVertices() const;
    std::span<const std::uint32_t> GetIndices() const;
    std::string_view GetModelName() const;

    std::string GetLastErrorMessage() const;

    std::runtime_error GetError() const;

    bool HasErrorOccured() const;

    std::uint32_t GetNumIndices() const;
    std::uint32_t GetNumVertices() const;

private:
    std::unique_ptr<IStaticMeshLoader> _staticMeshLoader;
    std::string _errorMessage;
    bool _errorOccured : 1;
};