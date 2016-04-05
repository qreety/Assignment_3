//[VERTEX SHADER]
#version 330

layout (location = 0) in vec3 InVertex;
layout (location = 1) in vec3 InNormal;
layout (location = 2) in int m;

smooth out vec3 vNormal;
smooth out vec3 vPos;
flat out int mindex;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;

void main()
{
	gl_Position = MVP * vec4(InVertex, 1.0);
	vPos = (V * vec4(InVertex, 1)).xyz;
	vNormal = mat3(transpose(inverse(M))) * InNormal;
	mindex = m;
}