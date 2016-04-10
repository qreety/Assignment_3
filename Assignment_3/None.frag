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

in vec3 vPos;
in vec3 vNormal;

out vec4 color;

uniform Material material;
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
        
    vec3 result = ambient + diffuse + specular;
    color = vec4(result, 1.0f);
}