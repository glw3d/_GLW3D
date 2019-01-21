
#version 110 // Specify which version of GLSL we are using.

/*
    Simple example shader
*/

uniform vec4 PrimaryColor;

varying vec3 color;

void main(void)
{   
    float a = PrimaryColor.a;
    float r = color.r * (1.0-a) + PrimaryColor.r * a;
    float g = color.g * (1.0-a) + PrimaryColor.g * a;
    float b = color.b * (1.0-a) + PrimaryColor.b * a;

    out_Color = vec4 (r, g, b, 1.0);
}

