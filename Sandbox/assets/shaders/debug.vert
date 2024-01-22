#version 430 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in uint a_color;

uniform mat4 u_projection_view;
uniform mat4 u_transform;
uniform mat3 u_normal_transform;

out vec4 color;

void main() {
    vec3 frag_pos_ws = vec3(u_transform * vec4(a_position, 1));
    gl_Position = u_projection_view * vec4(frag_pos_ws, 1);
    color = vec4((a_color & 0xff) / 255.0f, ((a_color >> 8) & 0xff) / 255.0f,  ((a_color >> 16) & 0xff) / 255.0f,  ((a_color >> 24) & 0xff) / 255.0f);
}  