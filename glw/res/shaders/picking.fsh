/* [FRAG] */
#version 150 core // Specify which version of GLSL we are using.

uniform int PickingId;

flat in int vertex_id;

out int out_PanelID;
out int out_VertexID;

void main(void)
{  
    out_PanelID = PickingId;
    out_VertexID = vertex_id;
}

