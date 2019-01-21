
#version 150 core // Specify which version of GLSL we are using.

/*
    Simple shader for a spot lighting and texturing
*/

uniform sampler2D Texture0;

uniform vec4 PrimaryColor;
uniform float SelfIlumination;
uniform float TextureBlend;

// smooth, flat, noperspective
smooth in float Coszeta;
smooth in vec2 TexCoord;

out vec4 out_Color;

void main(void)
{
	vec4 tex0 = texture(Texture0, TexCoord);
    	
    float tb = TextureBlend;
    float tb1 = 1.0-tb;

    vec3 color;
	color.r = tex0.r * tb1 + PrimaryColor.r * tb;
	color.g = tex0.g * tb1 + PrimaryColor.g * tb;
	color.b = tex0.b * tb1 + PrimaryColor.b * tb;

	color *= max( Coszeta, SelfIlumination );
	out_Color = vec4 (color.r, color.g, color.b, PrimaryColor.a);
}
