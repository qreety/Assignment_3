#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 Color;

// Values that stay constant for the whole mesh.
uniform sampler2D text;
uniform sampler2D bump;

void main(){

	// Output color = color of the texture at the specified UV
	Color = texture2D( bump, UV.st ).rgb;
}