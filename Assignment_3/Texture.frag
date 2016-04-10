#version 330
struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shine;
};

struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	bool on;
};

struct Texture{
	sampler2D text;
	bool on;
};

#define MAT_COUNT 6

in vec3 vPos;
in vec3 vNormal;
in vec2 UV;

out vec4 color;

uniform Material material;
uniform Texture wall;
uniform Light light;
uniform vec3 camPos;

void main()
{
	// Ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // Diffuse 
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    
    // Specular
    vec3 viewDir = normalize(camPos - vPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    vec3 specular = light.specular * (spec * material.specular); 

	if(wall.on)
		color =  texture2D(wall.text, UV) + vec4(vec3(texture2D(wall.text, UV)) * (ambient + diffuse) + specular, 1.0f);
	else
		color = vec4(ambient + diffuse + specular, 1.0f);
}