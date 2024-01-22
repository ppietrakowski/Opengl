#version 430 core

layout(location = 0) in vec3 a_position;

out vec3 position;

uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
	vec4 pos = u_projection * u_view * vec4(a_position, 1);
	gl_Position = pos.xyww;
	position = a_position;
}