#version 430 core

struct Material {
    sampler2D diffuse1;
    sampler2D diffuse2;
    float reflection_factor;
    float shininess;
};

in flat uint textureId;
in vec2 textureCoords;
in vec3 frag_pos_ws;
in vec3 normal;

uniform Material u_material;
uniform vec3 u_camera_location;
uniform samplerCube u_skybox_texture; 

struct Light {
    vec3 position;
    vec3 Direction;
    vec3 Color;
    float DirectionLength;
    int type;
    float cutoff;
    float Intensity;
};

layout(std140, binding=0) uniform Lights {
    Light u_lights[32];
};

const int kLightTypeDirectional = 0;
const int kLightTypePoint = 1;
const int kLightTypeSpot = 2;

uniform int u_num_lights;


// Calculates Color of fragment when specific light illuminates it. Calculation are using Phong shading model
vec3 CalculateLight(Light light, vec3 norm, vec3 view_dir, vec4 texel, vec3 reflect_skybox) {
    vec3 ambient = 0.01f * texel.xyz;

    // Diffuse lighting
    vec3 light_dir = normalize(-light.Direction);

    // __________
    // cos(theta)
    float diff = max(dot(light_dir, norm), 0.0f);

    vec3 diffuse = light.Intensity * texel.xyz * light.Color * diff;

    // Specular lighting calculation
    vec3 specular = vec3(0.0, 0.0, 0.0);

    if (dot(light_dir, view_dir) > 0.0) {
        vec3 refl = reflect(-light_dir, norm);
        float fi = dot(view_dir, refl);
        
        specular = light.Intensity * light.Color * pow(max(0.0, fi), u_material.shininess);
    }

    if (light.type == kLightTypePoint) {
        float attenuation = 0;
        float dist = distance(light.position, frag_pos_ws);

        if (dist <= 0.01f) {
            attenuation = 1;
        } else if (dist > 0.01f && dist < light.DirectionLength) {
            attenuation = (light.DirectionLength - dist) / (light.DirectionLength - 0.01f);
        }

        diffuse *= attenuation;
        specular *= attenuation;
    } else if (light.type == kLightTypeSpot) {
        vec3 light_to_fragment = normalize(frag_pos_ws - light.position);
        float spot_factor = dot(light_to_fragment, light.Direction);

        if (spot_factor > light.cutoff) {
            float attenuation = 0;
            float dist = distance(light.position, frag_pos_ws);

            if (dist <= 0.01f) {
                attenuation = 1;
            } else if (dist > 0.01f && dist < light.DirectionLength) {
                attenuation = (light.DirectionLength - dist) / (light.DirectionLength - 0.01f);
            }

            diffuse *= attenuation;
            specular *= attenuation;
        } else {
            diffuse = vec3(0, 0, 0);
            specular = vec3(0, 0, 0);
        }
    }

    // Final Color calculation
    return clamp(ambient + diffuse + specular, 0.0, 1.0);
}

out vec4 frag_color;

void main() {
    vec4 texel = vec4(0, 0, 0, 1);

    if (textureId == 0u) {
        texel = texture(u_material.diffuse1, textureCoords);
    } else if (textureId == 1u) {
        texel = texture(u_material.diffuse2, textureCoords);
    }
    
    vec3 Color = vec3(0, 0, 0);
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(u_camera_location - frag_pos_ws);
    vec3 eye_dir = normalize(frag_pos_ws - u_camera_location);
    vec3 reflect_vec = reflect(eye_dir, normalize(norm));

    for (int i = 0; i < u_num_lights; ++i) {
        Color += CalculateLight(u_lights[i], norm, view_dir, texel, reflect_vec);
    }
    
    Color += vec3(u_material.reflection_factor * texture(u_skybox_texture, reflect_vec));

    frag_color = vec4(Color, 1.0);
}