#version 150 core // Specify which version of GLSL we are using.

/*
    Simple example shader
*/

uniform mat4 ModelViewProjMatrix; // Camera matrix
 
in vec3 in_Position;		
in vec3 in_Normal;			 
in vec2 in_TexCoord;			 

smooth out vec3 color;

void main(void)
{
	gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);

	color.r = in_Position.x + 0.5;
	color.g = in_Position.y + 0.5;
	color.b = in_Position.z + 0.5;
}

