#version 330 core

struct Material 
{
    vec3 diffuse;
};

uniform Material u_material;
out vec4 frag_color;

void main()
{
    frag_color = vec4(u_material.diffuse, 1.0);
}