#pragma once

#include "core.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/gtc/type_ptr.hpp>

FORCE_INLINE glm::mat4 ToGlm(const aiMatrix4x4& matrix)
{
    return glm::transpose(glm::make_mat4(matrix[0]));
}

FORCE_INLINE glm::vec3 ToGlm(const aiVector3D& v)
{
    return glm::vec3{v.x, v.y, v.z};
}

FORCE_INLINE glm::vec2 ToGlm(aiVector2D v)
{
    return glm::vec2{v.x, v.y};
}

FORCE_INLINE glm::quat ToGlm(const aiQuaternion& quat)
{
    return glm::quat{quat.w, quat.x, quat.y, quat.z};
}

constexpr inline uint32_t AssimpImportFlags = aiProcess_Triangulate |
aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs;