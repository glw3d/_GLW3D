/* [FRAG] */
#version 110 // Specify which version of GLSL we are using.

uniform int PickingId;

void main(void)
{
	float r = float(PickingId) / 255.0;
	float g = float(PickingId) / 255.0;
	float b = float(PickingId) / 255.0;

	gl_FragColor = vec4 (r, g, b, 1.0);
}

