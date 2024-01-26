#version 330 core

uniform samplerCube u_SkyboxTexture; 
in vec3 Position;

out vec4 FragColor;

void main() 
{
	FragColor = texture(u_SkyboxTexture, Position);
}