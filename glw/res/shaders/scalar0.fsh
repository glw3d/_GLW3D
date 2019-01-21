
#version 150 core // Specify which version of GLSL we are using.

/*
    Shader used to show an scalar value as a gradient of colors
*/

smooth in float Scalar;

out vec4 out_Color;

void main(void)
{
   // out_Color = vec4 (Scalar, 0, 0, 1);
   out_Color = vec4 (Scalar, 0, 0, 1);
}

