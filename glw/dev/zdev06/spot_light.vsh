#version 150 core // Specify which version of GLSL we are using.

/*
    Simple shader for a spot lighting and texturing
*/

uniform mat4 ViewMatrix;          // Camera matrix
uniform mat4 ModelMatrix;         // Transformation matrix
uniform vec3 LightDirection;      // This is actually the light position
uniform int Culling;

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;

smooth out float Coszeta;
smooth out vec2 TexCoord;

void main(void)
{
    mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
	gl_Position = ModelViewMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	vec3 normal = ( mat3(ModelMatrix) * vec3(in_Normal.x, in_Normal.y, in_Normal.z)).xyz;
	normal = normalize( normal );
	vec3 ld = normalize(gl_Position.xyz - LightDirection);
	Coszeta = dot( normal, ld );
	
	TexCoord = in_TexCoord;
	
	if (Culling < 0){
		Coszeta = -Coszeta;
	}
	if (Culling == 0 && Coszeta < 0){
		Coszeta = -Coszeta;
	}
	Coszeta = max( Coszeta, 0.0 );
}
