/* [FRAG] */
#version 110 // Specify which version of GLSL we are using.

/*
    This is a shader for blitting images on the screen.
*/

uniform sampler2D Texture0;
uniform vec4 PrimaryColor;
uniform vec4 SecondaryColor;

varying vec2 TexCoord;

void main(void)
{
	vec4 out_Color;  
	vec4 tex0;

	tex0 = texture2D(Texture0, TexCoord);

	float f = tex0.a;
	float f1 = 1.0 - f;
	float r = PrimaryColor.r * f;
	float g = PrimaryColor.g * f;
	float b = PrimaryColor.b * f;
	float a = max(f1, SecondaryColor.a);

	if( a < 0.05 ) discard;

	if (SecondaryColor.a > 0.05){
		r += SecondaryColor.r * f1;
		g += SecondaryColor.g * f1;
		b += SecondaryColor.b * f1;
	}

	gl_FragColor = vec4 (r, g, b, a);
}
