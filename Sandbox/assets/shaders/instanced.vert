#version 430 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TextureCoords;
layout (location = 3) in uint a_TextureId;

uniform mat4 u_ProjectionView;

layout(std140, binding=0) uniform Transforms 
{
    mat4 u_Transforms[400];
};

out vec2 TextureCoords;
out vec3 FragPosWS;
out vec3 Normal;
out flat uint TextureId;

void main() 
{
    FragPosWS = vec3(u_Transforms[gl_InstanceID] * vec4(a_Position, 1));
    gl_Position = u_ProjectionView * vec4(FragPosWS, 1);
    TextureCoords = a_TextureCoords;
    mat3 normalTransform = mat3(transpose(inverse(u_Transforms[gl_InstanceID])));

    Normal = normalize(normalTransform * a_Normal);
    TextureId = a_TextureId;
}  