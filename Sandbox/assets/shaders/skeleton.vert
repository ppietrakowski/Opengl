#version 440 core
layout (location = 0) in vec3 a_position; 
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coords;
layout (location = 3) in ivec4 a_bone_ids;
layout (location = 4) in vec4 a_bone_weights;
layout (location = 5) in uint a_texture_id;

out vec2 texture_coords;
out vec3 normal;
out vec3 frag_pos_ws;

out flat uint texture_id;

uniform mat4 u_bone_transforms[200];
uniform mat4 u_projection_view;
uniform mat4 u_transform;
uniform mat4 u_view;
uniform vec3 u_light_direction;
uniform vec3 u_light_position_ws;

void main() {
	vec4 weights = normalize(a_bone_weights);

	mat4 bone_transform = u_bone_transforms[int(a_bone_ids.x)] * weights.x;
	bone_transform  +=    u_bone_transforms[int(a_bone_ids.y)] * weights.y;	
	bone_transform  +=    u_bone_transforms[int(a_bone_ids.z)] * weights.z;
	bone_transform  +=    u_bone_transforms[int(a_bone_ids.w)] * weights.w;

	vec4 pos = bone_transform * vec4(a_position, 1.0);
	gl_Position = u_projection_view * u_transform * pos;
	frag_pos_ws = vec3(u_transform * bone_transform * pos);

	texture_coords = a_texture_coords;
	normal = normalize(mat3(transpose(inverse(u_transform * bone_transform))) * a_normal);
	texture_id = a_texture_id;
}
