#include "MeshLoader.h"
#include "ObjMeshLoader.h"

#include "Logging.h"

StaticMeshImporter::StaticMeshImporter(std::filesystem::path path)
{
    if (path.has_extension() && path.extension() == ".obj")
    {
        _staticMeshLoader.reset(new StaticObjMeshLoader());
    }

    if (_staticMeshLoader)
    {
        this->_errorOccured = !_staticMeshLoader->Load(path.string());
        this->_errorMessage = _staticMeshLoader->GetLastErrorMessage();
    }
    else
    {
        _errorOccured = true;
        _errorMessage = "Unknown mesh format";
    }

    if (_errorOccured)
    {
        ELOG_ERROR(LOG_ASSET_LOADING, "%s", _errorMessage.c_str());
    }
}

std::span<const StaticMeshVertex> StaticMeshImporter::GetVertices() const
{
    ASSERT(!_errorOccured);
    return _staticMeshLoader->GetVertices();
}

std::span<const std::uint32_t> StaticMeshImporter::GetIndices() const
{
    ASSERT(!_errorOccured);
    return _staticMeshLoader->GetIndices();
}

std::string_view StaticMeshImporter::GetModelName() const
{
    ASSERT(!_errorOccured);
    return _staticMeshLoader->GetModelName();
}

std::string StaticMeshImporter::GetLastErrorMessage() const
{
    return _errorMessage;
}

std::runtime_error StaticMeshImporter::GetError() const
{
    return std::runtime_error(_errorMessage.c_str());
}

bool StaticMeshImporter::HasErrorOccured() const
{
    return _errorOccured;
}

std::uint32_t StaticMeshImporter::GetNumIndices() const
{
    ASSERT(!_errorOccured);
    return static_cast<std::uint32_t>(_staticMeshLoader->GetIndices().size());
}

std::uint32_t StaticMeshImporter::GetNumVertices() const
{
    ASSERT(!_errorOccured);
    return static_cast<std::uint32_t>(_staticMeshLoader->GetVertices().size());
}
