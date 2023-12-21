#version 430 core

struct Material {
    sampler2D diffuse[2];
};

in flat uint texture_id;
in vec2 texture_coords;

uniform Material u_material;
out vec4 frag_color;

void main() {
    frag_color = texture(u_material.diffuse[texture_id], texture_coords);
}