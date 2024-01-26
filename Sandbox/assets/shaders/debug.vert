#version 430 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in uint a_Color;

out vec4 Color;

uniform mat4 u_ProjectionView;

void main() 
{
    gl_Position = u_ProjectionView * vec4(a_Position, 1);
    Color = vec4((a_Color & 0xff) / 255.0f, ((a_Color >> 8) & 0xff) / 255.0f,  ((a_Color >> 16) & 0xff) / 255.0f,  ((a_Color >> 24) & 0xff) / 255.0f);
}  