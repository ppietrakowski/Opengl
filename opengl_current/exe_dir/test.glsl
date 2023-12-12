[Vertex]

#version 330

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TextureCoord;
layout (location = 2) in vec3 a_Normal;

uniform mat4 u_ProjectionView;
uniform mat4 u_Transform;

uniform vec4 u_LightColor;

out vec2 TextureCoord0;
out vec4 FragColour;
out vec3 Normal0;
out vec3 CurrentPosition0;

void main()
{
	gl_Position = u_ProjectionView * u_Transform * vec4(a_Position, 1);
    TextureCoord0 = a_TextureCoord;
	FragColour = u_LightColor;
	Normal0 = a_Normal;
	CurrentPosition0 = vec3(u_Transform * vec4(a_Position, 1));
}

[Fragment]

#version 330

out vec4 color;
in vec2 TextureCoord0;
in vec4 FragColour;

in vec3 Normal0;
in vec3 CurrentPosition0;

struct Material
{
	vec4 Tint;
	sampler2D Texture;
};

uniform Material u_Material;
uniform vec3 u_LightPosition;
uniform vec3 u_CameraLocation;

void main()
{
	vec3 Normal = normalize(Normal0);
	vec3 LightDirection = normalize(u_LightPosition - CurrentPosition0);
	float Ambient = 0.2f;

	float Diffuse = max(dot(Normal, LightDirection), 0.0f);

	float SpecularLight = 0.5f;
	vec3 ViewDirection = normalize(u_CameraLocation - CurrentPosition0);
	vec3 ReflectionDirection = reflect(-ViewDirection, Normal0);
	float SpecularAmount = pow(max(dot(ViewDirection, ReflectionDirection), 0.0f), 8); 
	float Specular = SpecularAmount * SpecularLight;

    color = vec4(vec3(texture(u_Material.Texture, TextureCoord0) * FragColour * (Diffuse + Ambient + Specular)), 1);
}