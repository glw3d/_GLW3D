#version 110 // Specify which version of GLSL we are using.

/*
    Simple example shader
*/

uniform mat4 ModelViewProjMatrix; // Camera matrix
 
attribute vec3 in_Position;		
attribute vec3 in_Normal;			 
attribute vec2 in_TexCoord;			 

varying vec3 color;

void main(void)
{
    gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);

	color.x = in_Position.x + 0.5;
	color.y = in_Position.y + 0.5;
	color.z = in_Position.z + 0.5;
}

