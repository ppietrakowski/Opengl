#version 430 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in uint a_color;

out vec4 color;

uniform mat4 u_ProjectionView;

void main() {
    gl_Position = u_ProjectionView * vec4(a_position, 1);
    color = vec4((a_color & 0xff) / 255.0f, ((a_color >> 8) & 0xff) / 255.0f,  ((a_color >> 16) & 0xff) / 255.0f,  ((a_color >> 24) & 0xff) / 255.0f);
}  