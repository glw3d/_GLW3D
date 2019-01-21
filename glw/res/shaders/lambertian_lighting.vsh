#version 150 core // Specify which version of GLSL we are using.

/*
    A simple shader for Lambertian lighting
*/

uniform mat4 ModelViewProjMatrix;
uniform vec3 LightDirection;
uniform int Culling;
 
in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;

smooth out float Coszeta;

void main(void)
{
	gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	vec3 normal = ( mat3(ModelViewProjMatrix) * vec3(in_Normal.x, in_Normal.y, in_Normal.z)).xyz;
	normal = normalize( normal );
	vec3 ld = normalize( LightDirection );
	Coszeta = dot( normal, ld );
	
	if (Culling < 0){
		Coszeta = -Coszeta;
	}
	if (Culling == 0 && Coszeta < 0){
		Coszeta = -Coszeta;
	}

	Coszeta = max( Coszeta, 0.0 );

	
	//cos(zeta)^n (n>1) Creates more especular light
	//Coszeta *= Coszeta;
	//Coszeta *= Coszeta;
	//Coszeta = sqrt(Coszeta);
}
