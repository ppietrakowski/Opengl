#version 430 core

struct Material 
{
    sampler2D diffuse1;
    sampler2D diffuse2;
};

in flat uint TextureID;
in vec2 TextureCoords;

uniform Material u_Material;
out vec4 OutColor;

void main()
{
    if (TextureID == 0u) 
    {
        OutColor = texture(u_Material.diffuse1, TextureCoords);
    }
    else if (TextureID == 1u)
    {
        OutColor = texture(u_Material.diffuse2, TextureCoords);
    }
}