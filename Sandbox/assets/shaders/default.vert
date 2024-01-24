#version 430 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coords;
layout (location = 3) in uint a_texture_id;

uniform mat4 u_projection_view;
uniform mat4 u_transform;
uniform mat3 u_normal_transform;

out vec2 textureCoords;
out vec3 frag_pos_ws;
out vec3 normal;
out flat uint textureId;

void main() {
    frag_pos_ws = vec3(u_transform * vec4(a_position, 1));
    gl_Position = u_projection_view * vec4(frag_pos_ws, 1);
    textureCoords = a_texture_coords;
    normal = u_normal_transform * a_normal;
    textureId = a_texture_id;
}  