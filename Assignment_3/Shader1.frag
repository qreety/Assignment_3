#version 330 core
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



// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 vPos;
in vec3 vNormal;

// Ouput data
out vec4 Color;

// Values that stay constant for the whole mesh.
uniform sampler2D text;
uniform sampler2D bump1;
uniform sampler2D bump2;
uniform Material mat;
uniform Light light;
uniform vec3 camPos;

void main(){
	// Ambient
    vec3 ambient = light.ambient * vec3(texture(text, UV));
  	
    // Diffuse 
    vec3 newnorm;
	vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * mat.diffuse;  
    
    // Specular
    vec3 viewDir = normalize(camPos - vPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shine);
    vec3 specular = light.specular * spec * mat.specular;
        
    Color = vec4(vec3(texture(text, UV)) + ambient + diffuse + specular, 1.0f);
}