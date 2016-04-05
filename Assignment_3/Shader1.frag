#version 330
struct Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shine;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	bool on;
};

struct PointLight {
    vec3 position;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	bool on;
};

#define MAT_COUNT 6

flat in int mindex;
smooth in vec3 vPos;
smooth in vec3 vNormal;

out vec4 color;

uniform Material mat[MAT_COUNT];
uniform vec3 aLight;
uniform DirLight light;
uniform PointLight dLight;
uniform vec3 camPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(camPos - vPos);
	vec3 am = aLight * mat[mindex].ambient;
	vec3 result = am;
	
	if(light.on && dLight.on)
	{
	result = am + CalcDirLight(light, norm, viewDir) + CalcPointLight(dLight, norm, vPos, viewDir);
	}
	else if(light.on)
	{
	result = am + CalcDirLight(light, norm, viewDir);
	}
	else if (dLight.on)
	{
	result = am + CalcPointLight(dLight, norm, vPos, viewDir);
	}

	color = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 halfDir = normalize( lightDir + camPos );  
    float spec = pow(max(dot( halfDir, normal), 0.0), mat[mindex].shine);
    // Combine results
    vec3 ambient = light.ambient * mat[mindex].diffuse;
    vec3 diffuse = light.diffuse * diff * mat[mindex].diffuse;
    vec3 specular = light.specular * spec * mat[mindex].specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - vPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 halfDir = normalize( lightDir + camPos );  
    float spec = pow(max(dot( halfDir, normal), 0.0), mat[mindex].shine);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f;    
    // Combine results
    vec3 ambient = light.ambient * mat[mindex].diffuse;
    vec3 diffuse = light.diffuse * diff * mat[mindex].diffuse;
    vec3 specular = light.specular * spec * mat[mindex].specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}