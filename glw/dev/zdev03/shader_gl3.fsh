
#version 150 core // Specify which version of GLSL we are using.

/*
    Simple example shader
*/

uniform vec4 PrimaryColor;

smooth in vec3 color;	// smooth, flat, noperspective

out vec4 out_Color;

void main(void)
{	
    float a = PrimaryColor.a;
    float r = color.r * (1.0-a) + PrimaryColor.r * a;
    float g = color.g * (1.0-a) + PrimaryColor.g * a;
    float b = color.b * (1.0-a) + PrimaryColor.b * a;

    out_Color = vec4 (r, g, b, 1.0);
}

