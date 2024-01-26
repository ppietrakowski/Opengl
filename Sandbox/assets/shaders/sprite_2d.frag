#version 330 core

uniform sampler2D u_Textures[32];

in vec4 Tint;
in vec2 TextureCoords;
flat in int TextureId;

out vec4 FragColor;

void main() 
{
	FragColor = Tint * texture(u_Textures[TextureId], TextureCoords);
}