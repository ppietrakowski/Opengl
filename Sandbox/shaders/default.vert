#version 430 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TextureCoords;
layout (location = 3) in uint a_TextureID;

uniform mat4 u_ProjectionView;
uniform mat4 u_Transform;
uniform mat3 u_NormalTransform;

out vec2 TextureCoords;
out vec3 FragmentPositionWS;
out vec3 Normal;
out flat uint TextureID;

void main()
{
    FragmentPositionWS = vec3(u_Transform * vec4(a_Position, 1));

    gl_Position = u_ProjectionView * vec4(FragmentPositionWS, 1);
    TextureCoords = a_TextureCoords;
    Normal = u_NormalTransform * a_Normal;
    TextureID = a_TextureID;
}  