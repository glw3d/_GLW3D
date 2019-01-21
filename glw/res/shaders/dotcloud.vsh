#version 150 core // Specify which version of GLSL we are using.

uniform mat4 ModelViewProjMatrix; 

in vec3 in_Position;

void main(void)
{
	gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
}
