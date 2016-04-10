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
	sampler2D texture;
	bool on;
};

in vec3 vPos;
in vec3 vNormal;
in vec2 UV;

out vec4 color;

uniform Material material;
uniform Texture wall;
uniform Texture bump1;
uniform Texture bump2;
uniform Light light;
uniform vec3 camPos;

void main()
{
	vec3 result;
	if(!light.on){
		if(wall.on)
			color = texture2D(wall.texture, UV);
		else
			color = vec4(0);
	}
	else{
	// Ambient
    vec3 ambient = light.ambient * material.ambient;
  	
    // Diffuse 
	vec3 norm;
	if(bump1.on)
		norm = normalize( vec3(texture2D(bump1.texture, UV)) * 2 - 1 + vNormal );
	else if(bump2.on)
		norm = normalize( vec3(texture2D(bump2.texture, UV)) * 2 - 1 + vNormal );
	else
		norm = normalize(vNormal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    
    // Specular
    vec3 viewDir = normalize(camPos - vPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    vec3 specular = light.specular * (spec * material.specular); 

	if(wall.on)
		color =  texture2D(wall.texture, UV) + vec4(ambient + diffuse + specular, 1.0f);
	else
		color = vec4(ambient + diffuse + specular, 1.0f);
	}
}