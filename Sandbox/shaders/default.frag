#version 330 core

in vec2 TextureCoords;
in vec3 FragmentPositionWS;
in vec3 FragmentPositionVWS;
in vec3 Normal;

struct Material 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material u_Material;
uniform vec3 u_LightPos;
uniform vec3 u_LightColor;
uniform vec3 u_CameraLocation;

out vec4 OutColor;

void main()
{
    // Ambient lighting
    vec3 ambient = u_Material.ambient * u_LightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_LightPos - FragmentPositionWS);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = u_Material.diffuse * u_LightColor * diff;

    // Specular lighting
    vec3 viewDir = normalize(u_CameraLocation -FragmentPositionWS);

    vec3 specular = vec3( 0.0, 0.0, 0.0 );
    if( dot( lightDir, viewDir ) > 0.0 )
    {
	    vec3 refl = reflect( vec3( 0.0, 0.0, 0.0 ) - lightDir, norm );
	    specular = pow( max( 0.0, dot( viewDir, refl ) ), u_Material.shininess ) * u_Material.specular;
    }

    // Final color calculation
    vec3 result = clamp( ambient + diffuse + specular, 0.0, 1.0 );
    OutColor = vec4(result, 1.0);
}