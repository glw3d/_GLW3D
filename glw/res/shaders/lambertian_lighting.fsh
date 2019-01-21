
#version 150 core // Specify which version of GLSL we are using.

/*
    Simple shader for simple Lambertian lighting
*/

uniform vec4 PrimaryColor;
uniform vec4 AmbientLight;
uniform float SelfIlumination;

smooth in float Coszeta;	// smooth, flat, noperspective

out vec4 out_Color;

void main(void)
{
	vec3 color = vec3
		( PrimaryColor.r * AmbientLight.r
		, PrimaryColor.g * AmbientLight.g
		, PrimaryColor.b * AmbientLight.b 
		);
	float self_ilumination = SelfIlumination;
	float norm = Coszeta * (1.0 - AmbientLight.a) + AmbientLight.a;
	color *= max( norm, self_ilumination );
	
	out_Color = vec4 (color.r, color.g, color.b, PrimaryColor.a);
}
