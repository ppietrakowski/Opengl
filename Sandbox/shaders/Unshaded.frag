#version 330 core

struct Material 
{
    vec3 diffuse;
};

uniform Material u_Material;
out vec4 OutColor;

void main()
{
    OutColor = vec4(u_Material.diffuse, 1.0);
}