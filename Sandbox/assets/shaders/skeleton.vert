#version 440 core
layout (location = 0) in vec3 a_Position; 
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TextureCoords;
layout (location = 3) in ivec4 a_BoneIds;
layout (location = 4) in vec4 a_BoneWeights;
layout (location = 5) in uint a_TextureId;

out vec2 TextureCoords;
out vec3 Normal;
out vec3 FragPosWS;

out flat uint TextureId;

uniform mat4 u_BoneTransforms[200];
uniform mat4 u_ProjectionView;
uniform mat4 u_Transform;

void main() 
{
	vec4 weights = normalize(a_BoneWeights);

	mat4 transformFromBones = u_BoneTransforms[int(a_BoneIds.x)] * weights.x;
	transformFromBones  +=    u_BoneTransforms[int(a_BoneIds.y)] * weights.y;	
	transformFromBones  +=    u_BoneTransforms[int(a_BoneIds.z)] * weights.z;
	transformFromBones  +=    u_BoneTransforms[int(a_BoneIds.w)] * weights.w;

	vec4 pos = transformFromBones * vec4(a_Position, 1.0);
	gl_Position = u_ProjectionView * u_Transform * pos;
	FragPosWS = vec3(u_Transform * transformFromBones * pos);

	TextureCoords = a_TextureCoords;
	Normal = normalize(mat3(transpose(inverse(u_Transform * transformFromBones))) * a_Normal);
	TextureId = a_TextureId;
}
