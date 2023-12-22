#version 430 core

struct Material {
    sampler2D diffuse1;
    sampler2D diffuse2;
};

in flat uint texture_id;
in vec2 texture_coords;
in vec3 frag_pos_ws;
in vec3 normal;
in vec3 eye_direction;
in vec3 light_direction;
in float dist;

uniform Material u_material;
out vec4 frag_color;

void main() {

  // Normal of the computed fragment, in camera space
  vec3 n = normalize( normal );

  // Direction of the light (from the fragment to the light)
  vec3 l = normalize( light_direction );
  float cos_theta = clamp( dot( n,l ), 0.4,1.0 );

  vec4 texel = vec4(0, 0, 0, 1);

  if (texture_id == 0u) {
      texel = texture(u_material.diffuse1, texture_coords);
  } else if (texture_id == 1u) {
      texel = texture(u_material.diffuse2, texture_coords);
  }

  frag_color = texel * cos_theta / (dist * dist);
}