#version 460

layout(vertices = 4) out;


layout(location = 0) out int[] tc_entity_id;
layout(location = 1) out vec3[] tc_color;
layout(location = 2) out vec3[] tc_normal;
layout(location = 3) out vec2[] tc_uv;

layout(location = 0) in int[] v_entity_id;
layout(location = 1) in vec3[] v_color;
layout(location = 2) in vec3[] v_normal;
layout(location = 3) in vec2[] v_uv;

void main()
{
	tc_entity_id[gl_InvocationID] = v_entity_id[gl_InvocationID];
	tc_color[gl_InvocationID] = v_color[gl_InvocationID];
	tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
	tc_uv[gl_InvocationID] = v_uv[gl_InvocationID];

	if(gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = 16;	
		gl_TessLevelOuter[1] = 16;
		gl_TessLevelOuter[2] = 16;
		gl_TessLevelOuter[3] = 16;

		gl_TessLevelInner[0] = 16;
		gl_TessLevelInner[1] = 16;
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}