#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 vNormal;
out vec3 vPos;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(position,1);
	vPos = (V * vec4(position, 1)).xyz;
	vNormal = mat3(transpose(inverse(M))) * normal;
	// UV of the vertex. No special space for this one.
	UV = uv;
}

