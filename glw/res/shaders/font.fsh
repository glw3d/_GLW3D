/* [FRAG] */
#version 150 core // Specify which version of GLSL we are using.

/*
    This is a shader for blitting images on the screen.
*/

uniform sampler2D Texture0;
uniform vec4 PrimaryColor;
uniform vec4 SecondaryColor;

smooth in vec2 TexCoord;

out vec4 out_Color;

void main(void)
{
	vec4 tex0 = texture(Texture0, TexCoord);
	float f = tex0.a;
	float f1 = 1.0 - f;
	float r = PrimaryColor.r * f;
	float g = PrimaryColor.g * f;
	float b = PrimaryColor.b * f;
	float a = max(f, SecondaryColor.a);

	if( a < 0.05 ) discard;
	
	if (SecondaryColor.a > 0.05){
		r += SecondaryColor.r * f1;
		g += SecondaryColor.g * f1;
		b += SecondaryColor.b * f1;
	}
	
	out_Color.r = r;
	out_Color.g = g;
	out_Color.b = b;
	out_Color.a = a;
}
