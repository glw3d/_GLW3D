#version 110 // Specify which version of GLSL we are using.

uniform mat4 ModelViewProjMatrix; 

// With 'layout(location = x)' we can indicate the attribute location, 
// but Overflow works in a way that this is not required 
attribute vec3 in_Position;
attribute vec3 in_Normal;
attribute vec2 in_TexCoord;

void main(void)
{
	gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
}

