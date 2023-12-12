#include "ObjMeshLoader.h"
#include "Core.h"

#include <fstream>

void SplitString(const std::string& str, std::string_view delimiter, std::vector<std::string>& outTokens)
{
    std::size_t startOffset = 0;
    std::size_t posOfDelimiter = str.find(delimiter, startOffset);

    while (posOfDelimiter != std::string::npos)
    {
        outTokens.push_back(str.substr(startOffset, posOfDelimiter - startOffset));

        startOffset = posOfDelimiter + delimiter.length();
        posOfDelimiter = str.find(delimiter, startOffset);
    }

    outTokens.push_back(str.substr(startOffset));
}

#define OBJ_LOCATION_INDEX 0
#define OBJ_TEXTURE_COORD_INDEX 1
#define OBJ_NORMAL_INDEX 2

// Indices for mapping value from already loaded array to vertices
struct ObjIndices
{
    unsigned int IndexPosition = 0;
    unsigned int IndexTextureCoords = 0;
    unsigned int IndexNormal = 0;

    unsigned int IndicesID = 0;

    bool operator==(const ObjIndices& other) const
    {
        return IndexPosition == other.IndexPosition && IndexTextureCoords == other.IndexTextureCoords && IndexNormal == other.IndexNormal;
    }
};

bool StaticObjMeshLoader::Load(const std::string& path)
{
    std::vector<ObjIndices> indicesToVertex;

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
        if (line[0] == 's' || line[0] == '#' || line == "mtllib" || line == "usemtl")
        {
            continue;
        }
        else if (line[0] == 'o')
        {
            AssignNewName(line);
        }
        else if (ContainsString(line, "vt"))
        {
            std::vector<std::string> lines;
            SplitString(line, " ", lines);

            lines.erase(lines.begin());

            if (lines.size() != 2)
            {
                _lastErrorMessage = "Mesh vertex texture coords are corrupted";
                return false;
            }

            glm::vec2 vt = { std::stof(lines[0]), std::stof(lines[1]) };
            textureCoords.push_back(vt);
        }
        else if (ContainsString(line, "vn"))
        {
            std::vector<std::string> lines;
            SplitString(line, " ", lines);

            lines.erase(lines.begin());

            if (lines.size() != 3)
            {
                _lastErrorMessage = "Mesh vertex normals are corrupted";
                return false;
            }

            glm::vec3 normal = { std::stof(lines[0]), std::stof(lines[1]), std::stof(lines[2]) };
            normals.push_back(normal);
        }
        else if (line[0] == 'v')
        {
            std::vector<std::string> lines;
            SplitString(line, " ", lines);

            lines.erase(lines.begin());

            if (lines.size() != 3)
            {
                _lastErrorMessage = "Mesh vertex positions are corrupted";
                return false;
            }

            glm::vec3 position = { std::stof(lines[0]), std::stof(lines[1]), std::stof(lines[2]) };
            positions.push_back(position);
        }
        else if (ContainsString(line, "f"))
        {
            std::vector<std::string> faceElements;

            SplitString(line, " ", faceElements);

            // omit first element, because it contains tag of line
            faceElements.erase(faceElements.begin());

            if (faceElements.size() != 3)
            {
                _lastErrorMessage = "Mesh contains too much faces or is corrupted. Face should consist only from three vertices.";
                return false;
            }

            for (const std::string& face : faceElements)
            {
                std::vector<std::string> faceComponents;

                SplitString(face, "/", faceComponents);

                ObjIndices indices;

                // Any index in obj file is 1-index based, but we need to convert it to 0-based, means just subtract 1
                indices.IndexPosition = std::stoul(faceComponents[OBJ_LOCATION_INDEX]) - 1;
                indices.IndexTextureCoords = std::stoul(faceComponents[OBJ_TEXTURE_COORD_INDEX]) - 1;
                indices.IndexNormal = std::stoul(faceComponents[OBJ_NORMAL_INDEX]) - 1;

                indicesToVertex.emplace_back(indices);
            }
        }
    }

    unsigned int lastIndicesID = 0;

    for (std::size_t i = 0; i < indicesToVertex.size(); ++i)
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
            _indices.push_back(it->IndicesID);
        }
        else
        {
            // this index doesn't appear earlier, so it is possible to update vertices and indices
            ObjIndices& indexToVertex = indicesToVertex[i];
            indexToVertex.IndicesID = lastIndicesID;

            _indices.push_back(lastIndicesID);

            StaticMeshVertex vertex;

            vertex.Position = positions.at(indexToVertex.IndexPosition);
            vertex.TextureCoords = textureCoords.at(indexToVertex.IndexTextureCoords);
            vertex.Normal = normals.at(indexToVertex.IndexNormal);

            _vertices.push_back(vertex);
            lastIndicesID++;
        }
    }

    return true;
}

void StaticObjMeshLoader::AssignNewName(std::string& line)
{
    std::vector<std::string> names;

    SplitString(line, " ", names);

    _meshName = names.back();
}

std::span<const StaticMeshVertex> StaticObjMeshLoader::GetVertices() const
{
    return _vertices;
}

std::span<const unsigned int> StaticObjMeshLoader::GetIndices() const
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
