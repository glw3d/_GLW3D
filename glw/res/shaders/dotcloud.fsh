#version 150 core // Specify which version of GLSL we are using.

uniform vec4 PrimaryColor;

out vec4 out_Color;

void main(void)
{
	out_Color = vec4 (PrimaryColor.r, PrimaryColor.g, PrimaryColor.b, PrimaryColor.a);
}
