#version 430 core

in vec2 texture_coords;
in vec3 frag_pos_ws;
in vec3 normal;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material u_material;
uniform vec3 u_light_pos;
uniform vec3 u_light_color;
uniform vec3 u_camera_location;

out vec4 frag_color;

void main() {
    // Ambient lighting
    vec3 ambient = u_material.ambient * u_light_color;

    // Diffuse lighting
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(u_light_pos - frag_pos_ws);
    float diff = max(dot(light_dir, norm), 0.0);
    vec3 diffuse = u_material.diffuse * u_light_color * diff;

    // Specular lighting
    vec3 view_dir = normalize(u_camera_location - frag_pos_ws);

    vec3 specular = vec3(0.0, 0.0, 0.0);

    if (dot(light_dir, view_dir) > 0.0) {
	    vec3 refl = reflect(vec3(0.0, 0.0, 0.0) - light_dir, norm);
	    specular = pow(max(0.0, dot(view_dir, refl)), u_material.shininess) * u_material.specular;
    }

    // Final color calculation
    vec3 result = clamp(ambient + diffuse + specular, 0.0, 1.0);
    frag_color = vec4(result, 1.0);
}