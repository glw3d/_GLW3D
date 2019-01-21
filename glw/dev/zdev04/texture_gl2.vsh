#version 110 // Specify which version of GLSL we are using.

/*
    Simple shader for simple Lambertian lighting
*/

uniform mat4 ModelViewProjMatrix; // Camera matrix
uniform vec3 LightDirection;
uniform int Culling;
 
attribute vec3 in_Position;
attribute vec3 in_Normal;
attribute vec2 in_TexCoord;

varying float Coszeta;
varying vec2 TexCoord;

void main(void)
{
	gl_Position = ModelViewProjMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	mat4 m = ModelViewProjMatrix;
	mat3 matrix = mat3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
	vec3 normal = (matrix * in_Normal);
	normal = normalize( normal );
	vec3 ld = normalize( LightDirection );
	Coszeta = dot(normal, ld);

	if (Culling < 0){
		Coszeta = -Coszeta;
	}
	if (Culling == 0 && Coszeta < 0.0){
		Coszeta = -Coszeta;
	}
	Coszeta = max( Coszeta, 0.0 );
	
	TexCoord = in_TexCoord;
	
//	cos(zeta)^n Creates more especular light
//	Coszeta *= Coszeta;
//	Coszeta *= Coszeta;
//	Coszeta = sqrt(Coszeta);
}
