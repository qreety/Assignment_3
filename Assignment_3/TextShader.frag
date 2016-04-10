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

struct Texture {
	sampler2D texture;
	bool on;
};

// Interpolated values from the vertex shaders
in vec3 vPos;
in vec3 vNormal;
in vec2 UV;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

// Ouput data
out vec3 Color;

// Values that stay constant for the whole mesh.
uniform Texture wall;
uniform Texture bump1;
uniform Texture bump2;
uniform Material material;
uniform Light light;
uniform vec3 camPos;

vec3 None(Light light);
vec3 V(Light light);

void main(){   
    if(!wall.on && !bump1.on && !bump2.on)
		Color = None(light);
	if(wall.on && !bump1.on && !bump2.on)
		Color = V(light);
}

vec3 VBB(Light light){
	return vec3(0,0,0);
}

vec3 VB(Light light, Texture bump){
	return vec3(0,0,0);
}

vec3 B(Light light, Texture bump){
	return vec3(0,0,0);
}

vec3 V(Light light){
	// Ambient
    vec3 ambient = light.ambient * vec3(texture2D(wall.texture, UV));
  	
    // Diffuse
	vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;  
    
    // Specular
    vec3 viewDir = normalize(camPos - vPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    vec3 specular = light.specular * spec * material.specular;
        
    return vec3(texture2D(wall.texture, UV)) + ambient + diffuse + specular;
}

vec3 None(Light light){
	// Ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // Diffuse 
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // Specular
    vec3 viewDir = normalize(camPos - vPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    vec3 specular = light.specular * (spec * material.specular);  
        
    return ambient + diffuse + specular;
}

