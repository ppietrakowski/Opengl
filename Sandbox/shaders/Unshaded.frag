#version 330 core

struct Material 
{
    vec3 diffuse;
};

uniform Material u_material;

in vec4 color;
out vec4 frag_color;

void main()
{
    frag_color = color;
}