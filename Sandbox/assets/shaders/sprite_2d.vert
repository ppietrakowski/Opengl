#version 430 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texture_coords;
layout(location = 2) in int a_texture_index;
layout(location = 3) in uint a_color;

out vec4 tint;
out vec2 textureCoords;
flat out int textureId;

uniform mat4 u_projection;

void main() {
	gl_Position = u_projection * vec4(a_position, 0, 1);
	textureCoords = a_texture_coords;
	textureId = a_texture_index;
	tint = vec4((a_color & 0xff) / 255.0f, ((a_color >> 8) & 0xff) / 255.0f,  ((a_color >> 16) & 0xff) / 255.0f,  ((a_color >> 24) & 0xff) / 255.0f);
}