#version 430 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TextureCoords;
layout(location = 2) in int a_TextureId;
layout(location = 3) in uint a_RgbaColor;

out vec4 Tint;
out vec2 TextureCoords;
flat out int TextureId;

uniform mat4 u_Projection;

void main() 
{
	gl_Position = u_Projection * vec4(a_Position, 0.1, 1);
	TextureCoords = a_TextureCoords;
	TextureId = a_TextureId;
	Tint = vec4((a_RgbaColor & 0xff) / 255.0f, ((a_RgbaColor >> 8) & 0xff) / 255.0f, 
		((a_RgbaColor >> 16) & 0xff) / 255.0f,  ((a_RgbaColor >> 24) & 0xff) / 255.0f);
}