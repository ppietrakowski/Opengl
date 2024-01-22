#version 330 core

uniform samplerCube u_skybox_texture; 
in vec3 position;

out vec4 frag_color;

void main() {
	frag_color = texture(u_skybox_texture, position);
}