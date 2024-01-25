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
    vec3 color;
    float DirectionLength;
    int type;
    float cutoff;
    float Intensity;
    float OuterCutOff;
};

layout(std140, binding=0) uniform Lights {
    Light u_lights[32];
};

const int kLightTypeDirectional = 0;
const int kLightTypePoint = 1;
const int kLightTypeSpot = 2;

uniform int u_num_lights;

float CalculateAttentuation(vec3 lightPosition, float lightLength)
{
    float dist = distance(lightPosition, frag_pos_ws);
    float attenuation = 0.0f;

    if (dist <= 0.01f) 
    {
        attenuation = 1;
    } 
    else if (dist > 0.01f && dist < lightLength)
    {
        attenuation = (lightLength - dist) / (lightLength - 0.01f);
    }

    return attenuation;
}

vec3 CalculateSpecular(vec3 lightDir, vec3 viewDir, vec3 norm, Light light)
{
    // Specular lighting calculation
    vec3 specular = vec3(0.0, 0.0, 0.0);

    if (dot(lightDir, viewDir) > 0.0) 
    {
        vec3 refl = reflect(-lightDir, norm);
        float fi = dot(viewDir, refl);
        
        specular = light.Intensity * light.color * pow(max(0.0, fi), u_material.shininess);
    }

    return specular;
}

// Calculates color of fragment when specific light illuminates it. Calculation are using Phong shading model
vec3 CalculateLight(Light light, vec3 norm, vec3 view_dir, vec4 texel, vec3 reflect_skybox) 
{
    // Diffuse lighting

    // find light_dir, which points towards light 
    vec3 light_dir = vec3(0, 0, 0);

    if (light.type == kLightTypePoint)
    {
        // find light
        light_dir = normalize(light.position - frag_pos_ws);
    }
    else
    {
        // in spot light the position is included in it's lighting calculation
        light_dir = normalize(-light.Direction);
    }

    // __________
    // cos(theta)
    float diff = max(dot(light_dir, norm), 0.0f);

    vec3 diffuse = light.Intensity * texel.xyz * light.color * diff;

    vec3 specular = CalculateSpecular(light_dir, view_dir, norm, light);

    // apply attentuation, if can be applied
    if (light.type != kLightTypeDirectional) 
    {
        float attenuation = CalculateAttentuation(light.position, light.DirectionLength);
        diffuse *= attenuation;
        specular *= attenuation;
    } 

    if (light.type == kLightTypeSpot) 
    {
        vec3 light_to_fragment = normalize(frag_pos_ws - light.position);
        float spot_factor = dot(light_to_fragment, light.Direction);

        float epsilon = light.cutoff - light.OuterCutOff;
        float intensity = clamp((spot_factor - light.OuterCutOff) / epsilon, 0.0, 1.0);    

        diffuse *= intensity;
        specular *= intensity;
    }

    // Final color calculation
    return clamp(diffuse + specular, 0.0, 1.0);
}

out vec4 frag_color;

void main() {
    vec4 texel = vec4(0, 0, 0, 1);

    if (textureId == 0u) 
    {
        texel = texture(u_material.diffuse1, textureCoords);
    } 
    else if (textureId == 1u) 
    {
        texel = texture(u_material.diffuse2, textureCoords);
    }
    
    
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(u_camera_location - frag_pos_ws);
    vec3 eye_dir = normalize(frag_pos_ws - u_camera_location);
    vec3 reflect_vec = reflect(eye_dir, normalize(norm));

    // apply ambient lighting first
    vec3 color = 0.05f * texel.xyz;

    for (int i = 0; i < u_num_lights; ++i) 
    {
        color += CalculateLight(u_lights[i], norm, view_dir, texel, reflect_vec);
    }
    
    color += vec3(u_material.reflection_factor * texture(u_skybox_texture, reflect_vec));

    frag_color = vec4(color, 1.0);
}