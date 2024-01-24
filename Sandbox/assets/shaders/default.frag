#version 430 core

in vec2 textureCoords;
in vec3 frag_pos_ws;
in vec3 normal;

struct Material 
{
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

struct Light 
{
    vec3 Position;
    vec3 Direction;
    vec3 Color;
    float DirectionLength;
    int Type;
    float Cutoff;
    float Intensity;
};

layout(std140, binding=0) uniform Lights 
{
    Light u_lights[32];
};

const int kLightTypeDirectional = 0;
const int kLightTypePoint = 1;
const int kLightTypeSpot = 2;

uniform int u_num_lights;

out vec4 frag_color;

// Calculates Color of fragment when specific light illuminates it. Calculation are using Phong shading model
vec3 CalculateLight(Light light, vec3 norm, vec3 view_dir) 
{
    vec3 ambient = u_material.ambient;

    // Diffuse lighting
    vec3 light_dir = normalize(-light.Direction);

    // __________
    // cos(theta)
    float diff = max(dot(light_dir, norm), 0.0f);

    vec3 diffuse = light.Intensity * u_material.diffuse * light.Color * diff;

    // Specular lighting calculation
    vec3 specular = vec3(0.0, 0.0, 0.0);

    if (dot(light_dir, view_dir) > 0.0) 
    {
        vec3 refl = reflect(-light_dir, norm);
        float fi = dot(view_dir, refl);
        
        specular = light.Intensity * light.Color * pow(max(0.0, fi), u_material.shininess) * u_material.specular;
    }

    if (light.Type == kLightTypePoint) 
    {
        float attenuation = 0;
        float dist = distance(light.Position, frag_pos_ws);

        if (dist <= 0.01f) 
        {
            attenuation = 1;
        } 
        else if (dist > 0.01f && dist < light.DirectionLength)
        {
            attenuation = (light.DirectionLength - dist) / (light.DirectionLength - 0.01f);
        }

        diffuse *= attenuation;
        specular *= attenuation;
    } 
    else if (light.Type == kLightTypeSpot) 
    {
        vec3 light_to_fragment = normalize(frag_pos_ws - light.Position);
        float spot_factor = dot(light_to_fragment, light.Direction);

        if (spot_factor > light.Cutoff) 
        {
            float attenuation = 0;
            float dist = distance(light.Position, frag_pos_ws);

            if (dist <= 0.01f) 
            {
                attenuation = 1;
            } 
            else if (dist > 0.01f && dist < light.DirectionLength) 
            {
                attenuation = (light.DirectionLength - dist) / (light.DirectionLength - 0.01f);
            }

            diffuse *= attenuation;
            specular *= attenuation;
        } 
        else 
        {
            diffuse = vec3(0, 0, 0);
            specular = vec3(0, 0, 0);
        }
    }

    // Final Color calculation
    return clamp(ambient + diffuse + specular, 0.0, 1.0);
}

void main() 
{
    
    vec3 color = vec3(0, 0, 0);
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(u_camera_location - frag_pos_ws);
    vec3 eye_dir = normalize(frag_pos_ws - u_camera_location);
    vec3 reflect_vec = reflect(eye_dir, normalize(norm));

    for (int i = 0; i < u_num_lights; ++i) 
    {
        color += CalculateLight(u_lights[i], norm, view_dir);
    }

    color += vec3(u_material.reflection_factor * texture(u_skybox_texture, reflect_vec);

    frag_color = vec4(color, 1.0);
}