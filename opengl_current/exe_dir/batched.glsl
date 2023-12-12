[Vertex]

#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TextureCoord;

uniform mat4 u_ProjectionView;

out vec2 TextureCoord0;

void main()
{
	gl_Position = u_ProjectionView * vec4(a_Position, 1);
    TextureCoord0 = a_TextureCoord;
}


[Fragment]

#version 330

out vec4 color;
in vec2 TextureCoord0;

uniform sampler2D u_Texture;

void main()
{
    color = texture(u_Texture, TextureCoord0);
}