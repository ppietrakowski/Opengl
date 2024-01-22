#version 330 core

uniform sampler2D u_textures[32];

in vec4 tint;
in vec2 texture_coords;
flat in int texture_id;

out vec4 frag_color;

void main() {
	frag_color = tint * texture(u_textures[texture_id], texture_coords);
}