/* [FRAG] */
#version 150 core // Specify which version of GLSL we are using.

/*
    The fragment source for postprocessing.
	Actually is doing nothing in particular,
	but to forward the rendering. 
*/

uniform sampler2D Texture0;
uniform vec2 ScreenSize;   // width and height of the display (usually in pixels)

smooth in vec2 TexCoord;

out vec4 out_Color;

void main()
{
	float num_coarse_isolines = 10;
	float num_fine_isolines = 256;
	float dx = 1.0 / ScreenSize.x;
	float dy = 1.0 / ScreenSize.y;
	float c00 = texture( Texture0, vec2( TexCoord.x, TexCoord.y ) ).x * 5;
	
	vec4 v10 = texture( Texture0, vec2( TexCoord.x + dx, TexCoord.y ) );
	vec4 v11 = texture( Texture0, vec2( TexCoord.x, TexCoord.y + dy ) );
	vec4 v12 = texture( Texture0, vec2( TexCoord.x, TexCoord.y - dy ) );
	vec4 v13 = texture( Texture0, vec2( TexCoord.x - dx, TexCoord.y ) );
	
	float t = floor(c00);
	float f = c00 - t;
	float r, g, b;
	float a = 1;
	
	if (t >= 0.0 && t < 1.0){
		r = 1.0-f;
		g = 0.0;
		b = 1.0;
	}
	else if (t < 2.0){
		r = 0.0;
		g = f;
		b = 1.0;
	}
	else if (t < 3.0){
		r = 0.0;
		g = 1.0;
		b = 1.0-f;
	}
	else if (t < 4.0){
		r = f;
		g = 1.0;
		b = 0.0;
	}	
	else if (t < 5.0){
		r = 1.0;
		g = 1.0-f;
		b = 0.0;
	}
	else if (t == 5.0){
		r = 1.0;
		g = 0.0;
		b = 0.0;
	}
	else{
		a = 0;
	}
	
	float c10 = v10.x * num_coarse_isolines;
	float c11 = v11.x * num_coarse_isolines;
	float c12 = v12.x * num_coarse_isolines;
	float c13 = v13.x * num_coarse_isolines;

	float s0_coarse = abs(floor(c10) - floor(c13));
	float s1_coarse = abs(floor(c11) - floor(c12));
	
	c10 = v10.x * num_fine_isolines;
	c11 = v11.x * num_fine_isolines;
	c12 = v12.x * num_fine_isolines;
	c13 = v13.x * num_fine_isolines;

	float s0_fine = abs(floor(c10) - floor(c13));
	float s1_fine = abs(floor(c11) - floor(c12));

	if (s0_coarse + s1_coarse < 0.95 && s0_fine + s1_fine > 0.95){
		r *= 0.80;
		g *= 0.80;
		b *= 0.80;
	}
	
	out_Color.r = r;
	out_Color.g = g;
	out_Color.b = b;
	out_Color.a = a;

}
