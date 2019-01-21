
#version 110 // Specify which version of GLSL we are using.

/*
    Simple shader for simple Lambertian lighting
*/

uniform vec4 PrimaryColor;
uniform vec4 AmbientLight;
uniform float SelfIlumination;

varying float Coszeta;

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
	
	gl_FragColor = vec4 (color.r, color.g, color.b, 1.0);
	//gl_FragDepth = gl_FragCoord.z;
	//gl_FragColor = vec4 (gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, PrimaryColor.a);
}
