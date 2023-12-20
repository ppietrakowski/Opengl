#version 430 core

struct Material {
    sampler2D diffuse1;
    sampler2D diffuse2;
};

in flat uint texture_id;
in vec2 texture_coords;

uniform Material u_material;
out vec4 frag_color;

void main() {
    if (texture_id == 0u) {
        frag_color = texture(u_material.diffuse1, texture_coords);
    }
    else if (texture_id == 1u) {
        frag_color = texture(u_material.diffuse2, texture_coords);
    }
}