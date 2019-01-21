/* [VERT] */
#version 150 core // Specify which version of GLSL we are using.

/*
    Vertex source used for postprocessing
*/

in vec3 in_Position;
//in vec3 in_Normal; // Not used
in vec2 in_TexCoord;

smooth out vec2 TexCoord;

void main()
{
	gl_Position = vec4(2.0 * in_Position.x - 1.0, 2.0 * in_Position.y - 1.0, 0.0, 1.0);

    TexCoord.x = in_TexCoord.x;
    TexCoord.y = in_TexCoord.y;
}

