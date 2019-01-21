
#version 150 core // Specify which version of GLSL we are using.

/*
    Simple example shader
*/

uniform float Shade;

smooth in vec3 color;	// smooth, flat, noperspective

out vec4 out_Color;

void main(void)
{	
    out_Color = vec4 (color.r * Shade, color.g * Shade, color.b * Shade, 1.0);
}

