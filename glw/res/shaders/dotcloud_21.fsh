#version 110 // Specify which version of GLSL we are using.

uniform vec4 PrimaryColor;

void main(void)
{
	gl_FragColor = vec4 (PrimaryColor.r, PrimaryColor.g, PrimaryColor.b, PrimaryColor.a);
}
