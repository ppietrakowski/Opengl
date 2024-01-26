#version 430 core

struct Material 
{
    vec3 Ambient;
    vec3 Specular;
    vec3 Diffuse;
    float ReflectionFactor;
    float Shininess;
};

in flat uint TextureId;
in vec2 TextureCoords;
in vec3 FragPosWS;
in vec3 Normal;

uniform Material u_Material;
uniform vec3 u_CameraLocation;
uniform samplerCube u_SkyboxTexture; 

struct Light 
{
    vec3 Position;
    vec3 Direction;
    vec3 Color;
    float DirectionLength;
    int Type;
    float cutoff;
    float Intensity;
    float OuterCutOff;
};

layout(std140, binding=0) uniform Lights 
{
    Light u_Lights[32];
};

const int LightTypeDirectional = 0;
const int LightTypePoint = 1;
const int LightTypeSpot = 2;

uniform int u_NumLights;

float CalculateAttentuation(vec3 lightPosition, float lightLength)
{
    float dist = distance(lightPosition, FragPosWS);
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
        
        specular = light.Intensity * light.Color * u_Material.Specular * pow(max(0.0, fi), u_Material.Shininess);
    }

    return specular;
}

// Calculates color of fragment when specific light illuminates it. Calculation are using Phong shading model
vec3 CalculateLight(Light light, vec3 norm, vec3 viewDir) 
{
    // find lightDir, which points towards light 
    vec3 lightDir = vec3(0, 0, 0);

    if (light.Type == LightTypePoint)
    {
        lightDir = normalize(light.Position - FragPosWS);
    }
    else
    {
        // in spot light the position is included in it's lighting calculation
        lightDir = normalize(-light.Direction);
    }

    // __________
    // cos(theta)
    float diff = max(dot(lightDir, norm), 0.0f);

    vec3 diffuse = light.Intensity * u_Material.Diffuse * light.Color * diff;

    vec3 specular = CalculateSpecular(lightDir, viewDir, norm, light);

    // apply attentuation, if can be applied
    if (light.Type != LightTypeDirectional) 
    {
        float attenuation = CalculateAttentuation(light.Position, light.DirectionLength);
        diffuse *= attenuation;
        specular *= attenuation;
    } 

    if (light.Type == LightTypeSpot) 
    {
        vec3 lightToFragment = normalize(FragPosWS - light.Position);
        float spotFactor = dot(lightToFragment, light.Direction);

        float epsilon = light.cutoff - light.OuterCutOff;
        float intensity = clamp((spotFactor - light.OuterCutOff) / epsilon, 0.0, 1.0);    

        diffuse *= intensity;
        specular *= intensity;
    }

    // Final color calculation
    return clamp(diffuse + specular, 0.0, 1.0);
}

out vec4 FragColor;

void main() 
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(u_CameraLocation - FragPosWS);
    vec3 eyeDir = normalize(FragPosWS - u_CameraLocation);
    vec3 reflectDir = reflect(eyeDir, normalize(norm));

    // apply ambient lighting first
    vec3 color = u_Material.Ambient;

    for (int i = 0; i < u_NumLights; ++i) 
    {
        color += CalculateLight(u_Lights[i], norm, viewDir);
    }
    
    color += vec3(u_Material.ReflectionFactor * texture(u_SkyboxTexture, reflectDir));

    FragColor = vec4(color, 1.0);
}