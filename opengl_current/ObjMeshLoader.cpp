#include "ObjMeshLoader.h"
#include "Core.h"

#include <fstream>
#include <array>
#include <algorithm>

static std::vector<std::string> SplitString(const std::string& str, std::string_view delimiter)
{
    std::vector<std::string> tokens;
    std::size_t startOffset = 0;
    std::size_t posOfDelimiter = str.find(delimiter, startOffset);

    while (posOfDelimiter != std::string::npos)
    {
        tokens.push_back(str.substr(startOffset, posOfDelimiter - startOffset));

        startOffset = posOfDelimiter + delimiter.length();
        posOfDelimiter = str.find(delimiter, startOffset);
    }

    tokens.push_back(str.substr(startOffset));
    return tokens;
}

template <std::size_t N>
static std::array<std::string, N> SplitStringFixed(const std::string& str, std::string_view delimiter)
{
    std::array<std::string, N> tokens;
    std::size_t startOffset = 0;
    std::size_t posOfDelimiter = str.find(delimiter, startOffset);

    std::size_t index = 0;

    while (posOfDelimiter != std::string::npos)
    {
        tokens[index++] = str.substr(startOffset, posOfDelimiter - startOffset);

        startOffset = posOfDelimiter + delimiter.length();
        posOfDelimiter = str.find(delimiter, startOffset);

        if (index >= N - 1) { break; }
    }

    tokens[index] = str.substr(startOffset);
    return tokens;
}

#define OBJ_LOCATION_INDEX 0
#define OBJ_TEXTURE_COORD_INDEX 1
#define OBJ_NORMAL_INDEX 2

namespace obj
{
    // Indices for mapping value from already loaded array to vertices
    struct Face
    {
        std::uint32_t IndexPosition = 0;
        std::uint32_t IndexTextureCoords = 0;
        std::uint32_t IndexNormal = 0;

        std::uint32_t IndicesID = 0;

        bool operator==(const Face& other) const
        {
            return IndexPosition == other.IndexPosition && IndexTextureCoords == other.IndexTextureCoords && IndexNormal == other.IndexNormal;
        }

        Face(std::uint32_t indexPosition, std::uint32_t indexTextureCoords, std::uint32_t indexNormal) :
            IndexPosition{ indexPosition },
            IndexTextureCoords{ indexTextureCoords },
            IndexNormal{ indexNormal },
            IndicesID{ 0 }
        {
        }

        Face(const Face&) = default;
        Face& operator=(const Face&) = default;
    };
}

#define LINE_DEFINING_MESH_NAME(Line) (Line[0] == 'o')
#define LINE_DEFINING_TEXTURE_COORD(Line) (ContainsString(Line, "vt"))
#define LINE_DEFINING_MESH_NORMAL(Line) (ContainsString(Line, "vn"))
#define LINE_DEFINING_MESH_POSITION(Line) (Line[0] == 'v')
#define LINE_DEFINING_FACE(Line) (Line[0] == 'f')

bool StaticObjMeshLoader::Load(const std::string& path)
{
    std::vector<obj::Face> indicesToVertex;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textureCoords;

    std::ifstream file(path.c_str());

    if (!file)
    {
        _lastErrorMessage = "File access error";
        return false;
    }

    std::string line;

    _vertices.clear();
    _indices.clear();

    while (std::getline(file, line))
    {
        // # - comment/mtllib - material file/usetml - got material file
        if (line[0] == 's' || line[0] == '#' || line == "mtllib" || line == "usemtl")
        {
            continue;
        }
        else if (LINE_DEFINING_MESH_NAME(line))
        {
            AssignNewName(line);
        }
        else if (LINE_DEFINING_TEXTURE_COORD(line))
        {
            std::array<std::string, 3> lines = SplitStringFixed<3>(line, " ");

            // skip first (contains tag)
            glm::vec2 vt = { std::stof(lines[1]), std::stof(lines[2]) };
            textureCoords.push_back(vt);
        }
        else if (LINE_DEFINING_MESH_NORMAL(line))
        {
            std::array<std::string, 4> lines = SplitStringFixed<4>(line, " ");
            
            // skip first (contains tag)
            glm::vec3 normal = { std::stof(lines[1]), std::stof(lines[2]), std::stof(lines[3]) };
            normals.push_back(normal);
        }
        else if (LINE_DEFINING_MESH_POSITION(line))
        {
            std::array<std::string, 4> lines = SplitStringFixed<4>(line, " ");

            // skip first (contains tag)
            glm::vec3 position = { std::stof(lines[1]), std::stof(lines[2]), std::stof(lines[3]) };
            positions.push_back(position);
        }
        else if (LINE_DEFINING_FACE(line))
        {
            std::vector<std::string> faceElements = std::move(SplitString(line, " "));

            // omit first element, because it contains tag of line
            faceElements.erase(faceElements.begin());

            if (faceElements.size() != 3)
            {
                _lastErrorMessage = "Mesh contains too much faces or is corrupted. Face should consist only from three vertices.";
                return false;
            }

            for (const std::string& face : faceElements)
            {
                std::array<std::string, 3> faceComponents = std::move(SplitStringFixed<3>(face, "/"));
                std::array<std::uint32_t, 3> convertedFaceComponents;

                // convert all facecomponents to std::uint32_t and substract 1, because obj indices starts from 1
                std::transform(faceComponents.begin(), faceComponents.end(), convertedFaceComponents.begin(),
                    [](const std::string& component) { return std::stoul(component) - 1; });

                indicesToVertex.emplace_back(convertedFaceComponents[OBJ_LOCATION_INDEX],
                    convertedFaceComponents[OBJ_TEXTURE_COORD_INDEX], convertedFaceComponents[OBJ_NORMAL_INDEX]);
            }
        }
    }

    std::uint32_t lastIndicesID = 0;

    for (std::uint32_t i = 0; i < indicesToVertex.size(); ++i)
    {
        auto endIterator = indicesToVertex.begin() + i;

        // find first occurence of element on the list to find it was already defined
        auto it = std::find(indicesToVertex.begin(),
            endIterator, indicesToVertex[i]);

        bool alreadyDefined = it != endIterator;

        if (alreadyDefined)
        {
            // in that case just take indices number of already defined element
            indicesToVertex[i].IndicesID = it->IndicesID;
            _indices.emplace_back(it->IndicesID);
        }
        else
        {
            // this index doesn't appear earlier, so it is possible to update vertices and indices
            obj::Face& indexToVertex = indicesToVertex[i];
            indexToVertex.IndicesID = lastIndicesID;
            _indices.emplace_back(lastIndicesID);
            _vertices.emplace_back(positions.at(indexToVertex.IndexPosition), normals.at(indexToVertex.IndexNormal),
                textureCoords.at(indexToVertex.IndexTextureCoords));

            lastIndicesID++;
        }
    }

    return true;
}

void StaticObjMeshLoader::AssignNewName(std::string& line)
{
    std::vector<std::string> names = std::move(SplitString(line, " "));
    _meshName = names.back();
}

std::span<const StaticMeshVertex> StaticObjMeshLoader::GetVertices() const
{
    return _vertices;
}

std::span<const std::uint32_t> StaticObjMeshLoader::GetIndices() const
{
    return _indices;
}

std::string_view StaticObjMeshLoader::GetModelName() const
{
    return _meshName;
}

std::string StaticObjMeshLoader::GetLastErrorMessage() const
{
    return _lastErrorMessage;
}
