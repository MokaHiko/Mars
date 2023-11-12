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

struct ObjectData {
	vec4 color;
	mat4 model_matrix;
};

layout(set = 0, binding = 0) uniform GlobalBuffer {
	mat4 view;
	mat4 view_proj;

	vec4 camera_position;
	uint n_dir_lights;
} _global_buffer;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer{
	ObjectData s_objects[];
} _object_buffer;

const int MIN_TESS_LEVEL = 4;
const int MAX_TESS_LEVEL = 64;
const float MIN_DISTANCE = 20;
const float MAX_DISTANCE = 800;

void main()
{
	tc_entity_id[gl_InvocationID] = v_entity_id[gl_InvocationID];
	tc_color[gl_InvocationID] = v_color[gl_InvocationID];
	tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
	tc_uv[gl_InvocationID] = v_uv[gl_InvocationID];

	if(gl_InvocationID == 0)
	{
		// Step 1: Transform points into view space
		vec4 viewSpacePos00 = _global_buffer.view * _object_buffer.s_objects[v_entity_id[gl_InvocationID]].model_matrix * gl_in[0].gl_Position;
		vec4 viewSpacePos10 = _global_buffer.view * _object_buffer.s_objects[v_entity_id[gl_InvocationID]].model_matrix * gl_in[1].gl_Position;
		vec4 viewSpacePos01 = _global_buffer.view * _object_buffer.s_objects[v_entity_id[gl_InvocationID]].model_matrix * gl_in[2].gl_Position;
		vec4 viewSpacePos11 = _global_buffer.view * _object_buffer.s_objects[v_entity_id[gl_InvocationID]].model_matrix * gl_in[3].gl_Position;

		// Step 2: Get distance from camera clamped 
		float distance00 = clamp((abs(viewSpacePos00.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
		float distance10 = clamp((abs(viewSpacePos10.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
		float distance01 = clamp((abs(viewSpacePos01.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
		float distance11 = clamp((abs(viewSpacePos11.z) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);

		// Step 3: interpolate tesselation levels
		float tessLevel0 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance10));
		float tessLevel1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance11));
		float tessLevel2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance01));
		float tessLevel3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance00));

		gl_TessLevelOuter[0] = tessLevel0;	
		gl_TessLevelOuter[1] = tessLevel1;
		gl_TessLevelOuter[2] = tessLevel2;
		gl_TessLevelOuter[3] = tessLevel3;

		gl_TessLevelInner[0] = max(tessLevel0, tessLevel2);
		gl_TessLevelInner[1] = max(tessLevel1, tessLevel3);
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}