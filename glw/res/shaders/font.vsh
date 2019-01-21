/* [VERT] */
#version 150 core // Specify which version of GLSL we are using.

/*
    This is a shader for blitting images on the screen.
*/
uniform mat4 ModelViewProjMatrix; 

in vec3 in_Position;		
//in vec3 in_Normal; // Not used			 
in vec2 in_TexCoord;			 

smooth out vec2 TexCoord;

void main(void)
{
    gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
    gl_Position.y = gl_Position.y + 2;
    TexCoord.x = in_TexCoord.x;
    TexCoord.y = 1.0 - in_TexCoord.y;
}

