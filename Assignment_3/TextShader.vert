#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

// Output data ; will be interpolated for each fragment.
out vec3 vPos;
out vec2 UV;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out vec3 vNormal;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 camPos;
uniform vec3 lightDir;

void main(){

	gl_Position = MVP * vec4(position, 1.0f);
    vPos = vec3(M * vec4(position, 1.0));   
	vNormal = mat3(transpose(inverse(M))) * normal;
    UV = uv;
    
    mat3 normalMatrix = transpose(inverse(mat3(M)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vec3 N = normalize(normalMatrix * normal);    
    
    mat3 TBN = transpose(mat3(T, B, N));  
    TangentLightPos = TBN * (-lightDir);
	TangentViewPos  = TBN * camPos;
    TangentFragPos  = TBN * vPos;
}

