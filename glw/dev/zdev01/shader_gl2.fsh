
#version 110 // Specify which version of GLSL we are using.

/*
    Simple example shader
*/

uniform float Shade;

varying vec3 color;

void main(void)
{
	gl_FragColor = vec4 (color.r * Shade, color.g * Shade, color.b * Shade, 1.0);
}

