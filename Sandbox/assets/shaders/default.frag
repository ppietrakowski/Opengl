#version 430 core

in vec2 texture_coords;
in vec3 frag_pos_ws;
in vec3 normal;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float reflection_factor;
};

uniform Material u_material;
uniform vec3 u_light_pos;
uniform vec3 u_light_color;
uniform vec3 u_camera_location;
uniform mat4 u_view;
uniform samplerCube u_skybox_texture; 

struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    float direction_length;
    int type;
    float cutoff;
    float intensity;
};

layout(std140, binding=0) uniform Lights {
    Light u_lights[32];
};

const int kLightTypeDirectional = 0;
const int kLightTypePoint = 1;
const int kLightTypeSpot = 2;

uniform int u_num_lights;

out vec4 frag_color;

// Calculates color of fragment when specific light illuminates it. Calculation are using Phong shading model
vec3 CalculateLight(Light light, vec3 norm, vec3 view_dir) {
    vec3 ambient = u_material.ambient;

    // Diffuse lighting
    vec3 light_dir = normalize(-light.direction);

    // __________
    // cos(theta)
    float diff = max(dot(light_dir, norm), 0.0f);

    vec3 diffuse = light.intensity * u_material.diffuse * light.color * diff;

    // Specular lighting calculation
    vec3 specular = vec3(0.0, 0.0, 0.0);

    if (dot(light_dir, view_dir) > 0.0) {
        vec3 refl = reflect(-light_dir, norm);
        float fi = dot(view_dir, refl);
        
        specular = light.intensity * light.color * pow(max(0.0, fi), u_material.shininess) * u_material.specular;
    }

    if (light.type == kLightTypePoint) {
        float attenuation = 0;
        float dist = distance(light.position, frag_pos_ws);

        if (dist <= 0.01f) {
            attenuation = 1;
        } else if (dist > 0.01f && dist < light.direction_length) {
            attenuation = (light.direction_length - dist) / (light.direction_length - 0.01f);
        }

        diffuse *= attenuation;
        specular *= attenuation;
    } else if (light.type == kLightTypeSpot) {
        vec3 light_to_fragment = normalize(frag_pos_ws - light.position);
        float spot_factor = dot(light_to_fragment, light.direction);

        if (spot_factor > light.cutoff) {
            float attenuation = 0;
            float dist = distance(light.position, frag_pos_ws);

            if (dist <= 0.01f) {
                attenuation = 1;
            } else if (dist > 0.01f && dist < light.direction_length) {
                attenuation = (light.direction_length - dist) / (light.direction_length - 0.01f);
            }

            diffuse *= attenuation;
            specular *= attenuation;
        } else {
            diffuse = vec3(0, 0, 0);
            specular = vec3(0, 0, 0);
        }
    }

    // Final color calculation
    return clamp(ambient + diffuse + specular, 0.0, 1.0);
}

void main() {
    
    vec3 color = vec3(0, 0, 0);
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(u_camera_location - frag_pos_ws);
    vec3 eye_dir = normalize(frag_pos_ws - u_camera_location);
    vec3 reflect_vec = reflect(eye_dir, normalize(norm));

    for (int i = 0; i < u_num_lights; ++i) {
        color += CalculateLight(u_lights[i], norm, view_dir);
    }

    color += vec3(u_material.reflection_factor * texture(u_skybox_texture, reflect_vec);

    frag_color = vec4(color, 1.0);
}