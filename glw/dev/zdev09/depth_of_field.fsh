
#version 150 core // Specify which version of GLSL we are using.

/*
    Simple shader for postprocessing
*/

uniform sampler2D Texture0;
uniform sampler2D DepthBuffer;

smooth in vec2 TexCoord;

out vec4 out_Color;

void main(void)
{
	vec4 tex0 = texture(Texture0, TexCoord);
	float depth = texture(DepthBuffer, TexCoord).r;


	float f = depth;
	//out_Color = vec4 (tex0.r, tex0.g, tex0.b, 1.0 );
	out_Color = vec4 (f, f, f, 1.0 );
}
