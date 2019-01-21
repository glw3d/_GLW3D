/* [FRAG] */
#version 150 core // Specify which version of GLSL we are using.

/*
    This is a shader for blitting images on the screen.
*/

uniform sampler2D Texture0;
uniform vec4 PrimaryColor;
uniform float TextureBlend;

smooth in vec2 TexCoord;

out vec4 out_Color;

void main(void)
{
	vec4 tex0 = texture(Texture0, TexCoord);
	float alpha = PrimaryColor.w * tex0.w;
	if (alpha < 0.05)	
		discard;
	
	float tb = TextureBlend;
	float tb1 = 1-tb;
	
	out_Color.x = tex0.x * tb1 + PrimaryColor.x * tb;
	out_Color.y = tex0.y * tb1 + PrimaryColor.y * tb;
	out_Color.z = tex0.z * tb1 + PrimaryColor.z * tb;
	out_Color.w = alpha;
}

