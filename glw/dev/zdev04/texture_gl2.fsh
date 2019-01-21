
#version 150 core // Specify which version of GLSL we are using.

/*
    Simple shader for simple Lambertian lighting
*/

uniform sampler2D Texture0;

uniform vec4 AmbientLight;
uniform vec4 PrimaryColor;
uniform float TextureBlend;

// smooth, flat, noperspective
varying float Coszeta;
varying vec2 TexCoord;

void main(void)
{
	vec3 color;
	vec4 tex0 = texture(Texture0, TexCoord);
	float tb = TextureBlend;
	float tb1 = 1.0-tb;
	
	color.r = (tex0.r * tb + PrimaryColor.r * tb1) * AmbientLight.r;
	color.g = (tex0.g * tb + PrimaryColor.g * tb1) * AmbientLight.g;
	color.b = (tex0.b * tb + PrimaryColor.b * tb1) * AmbientLight.b;
	
	float self_ilumination = 1.0 - PrimaryColor.a;
	float norm = Coszeta * (1.0 - AmbientLight.a) + AmbientLight.a;
	color *= max( norm, self_ilumination );
	
	gl_FragColor = vec4 (color.r, color.g, color.b, 1.0);
}
