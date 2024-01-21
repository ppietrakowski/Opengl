#version 430 core

struct Material {
    sampler2D diffuse1;
    sampler2D diffuse2;
};

in flat uint texture_id;
in vec2 texture_coords;
in vec3 frag_pos_ws;
in vec3 normal;

uniform Material u_material;
uniform vec3 u_camera_location;

struct Light
{
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


// Calculates color of fragment when specific light illuminates it. Calculation are using Phong shading model
vec3 CalculateLight(Light light, vec3 norm, vec3 view_dir, vec4 texel) {
    vec3 ambient = 0.01f * texel.xyz;

    // Diffuse lighting
    vec3 light_dir = normalize(-light.direction);

    // __________
    // cos(theta)
    float diff = max(dot(light_dir, norm), 0.0f);

    vec3 diffuse = light.intensity * texel.xyz * light.color * diff;

    // Specular lighting calculation
    vec3 specular = vec3(0.0, 0.0, 0.0);

    if (dot(light_dir, view_dir) > 0.0) {
	    vec3 refl = reflect(-light_dir, norm);
        float fi = dot(view_dir, refl);

	    specular = light.intensity * light.color * pow(max(0.0, fi), 32);
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

out vec4 frag_color;

void main() {
    vec4 texel = vec4(0, 0, 0, 1);

    if (texture_id == 0u) {
        texel = texture(u_material.diffuse1, texture_coords);
    } else if (texture_id == 1u) {
        texel = texture(u_material.diffuse2, texture_coords);
    }
    
    vec3 color = vec3(0, 0, 0);
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(u_camera_location - frag_pos_ws);
    
    for (int i = 0; i < u_num_lights; ++i) {
        color += CalculateLight(u_lights[i], norm, view_dir, texel);
    }
    
    frag_color = vec4(color, 1.0);
}