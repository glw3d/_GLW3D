/* [FRAG] */
#version 110 // Specify which version of GLSL we are using.

/*
    This is a shader for blitting images on the screen.
*/

uniform sampler2D Texture0;
uniform vec4 PrimaryColor;
uniform float TextureBlend;

varying vec2 TexCoord;

void main(void)
{
	vec4 out_Color;  
	vec4 tex0;

	tex0 = texture2D(Texture0, TexCoord);
			
	float alpha = PrimaryColor.w * tex0.w;
	if (alpha < 0.05)	
		discard;
		
	float tb = TextureBlend;
	float tb1 = 1.0-tb;
	
	out_Color.r = tex0.x * tb1 + PrimaryColor.x * tb;
	out_Color.g = tex0.y * tb1 + PrimaryColor.y * tb;
	out_Color.b = tex0.z * tb1 + PrimaryColor.z * tb;
	out_Color.w = alpha;
	
	gl_FragColor = vec4 (out_Color.r, out_Color.g, out_Color.b, out_Color.w);
}
