#version 150 core // Specify which version of GLSL we are using.

/*
    Shader used to show an scalar value as a gradient of colors
*/

uniform mat4 ModelViewProjMatrix;
uniform vec2 ScalarRange; // min and maximun values

in vec3 in_Position;
in float in_Scalar;


smooth out float Scalar;

void main(void)
{
	gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	float min = ScalarRange.x;
	float max = ScalarRange.y;
    Scalar = (in_Scalar - min) / (max - min);
}
