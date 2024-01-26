#version 430 core

layout(location = 0) in vec3 a_Position;

out vec3 Position;

uniform mat4 u_ProjectionView;

void main() 
{
	vec4 pos = u_ProjectionView * vec4(a_Position, 1);
	gl_Position = pos.xyww;
	Position = a_Position;
}