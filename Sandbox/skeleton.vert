#version 440 core
layout (location = 0) in vec3 position; 
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 boneIds;
layout (location = 4) in vec4 boneWeights;

out vec2 tex_cord;
out vec3 v_normal;
out vec3 v_pos;
out vec4 bw;

uniform mat4 bone_transforms[100];
uniform mat4 u_ProjectionView;
uniform mat4 u_Transform;

void main()
{
	bw = vec4(0);
	if(int(boneIds.x) == 1)
	bw.z = boneIds.x;
	//boneWeights = normalize(boneWeights);
	mat4 boneTransform  =  mat4(0.0);
	boneTransform  +=    bone_transforms[int(boneIds.x)] * boneWeights.x;
	boneTransform  +=    bone_transforms[int(boneIds.y)] * boneWeights.y;
	boneTransform  +=    bone_transforms[int(boneIds.z)] * boneWeights.z;
	boneTransform  +=    bone_transforms[int(boneIds.w)] * boneWeights.w;
	vec4 pos =boneTransform * vec4(position, 1.0);
	gl_Position = u_ProjectionView * u_Transform * pos;
	v_pos = vec3(u_Transform * boneTransform * pos);
	tex_cord = uv;
	v_normal = mat3(transpose(inverse(u_Transform * boneTransform))) * normal;
	v_normal = normalize(v_normal);
}
