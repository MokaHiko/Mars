#version 460

layout (quads, equal_spacing, cw) in;

layout(location = 0) in int[] tc_entity_id;
layout(location = 1) in vec3[] tc_color;
layout(location = 2) in vec3[] tc_normal;
layout(location = 3) in vec2[] tc_uv;

layout(location = 0) out vec3 tese_position_world_space;
layout(location = 1) out vec3 tese_color;
layout(location = 2) out vec2 tese_uv;

layout(location = 3) out vec3 tese_normal_world_space;
layout(location = 4) out vec4 tese_uv_world_space;

// TODO: Height map and scrolling ocean textures
// layout(location = 5) out float tese_height;
// layout(set = 3, binding = 0) uniform sampler2D _height_map_texture;

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

float Rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float yScale = 64.0f; 
float yShift = 16.0f;
void main()
{
	// Weights of the specific patch vertex in the generated quad
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec4 p00 = gl_in[0].gl_Position;
	vec4 p10 = gl_in[1].gl_Position;
	vec4 p01 = gl_in[2].gl_Position;
	vec4 p11 = gl_in[3].gl_Position;

	vec2 t00 = tc_uv[0];
	vec2 t10 = tc_uv[1];
	vec2 t01 = tc_uv[2];
	vec2 t11 = tc_uv[3];

	// Interpolate using weights
	vec4 p1 = mix(p00, p10, u);
	vec4 p2 = mix(p01, p11, u);
	vec4 p = mix(p1, p2, v);

	vec2 t0 = mix(t00, t10, u);
	vec2 t1 = mix(t01, t11, u);
	vec2 texCoords = mix(t0, t1, v);

	mat4 model_matrix = _object_buffer.s_objects[tc_entity_id[0]].model_matrix;
	tese_position_world_space = mat3(model_matrix) * vec3(p); // position in world space
	tese_normal_world_space = normalize(mat3(model_matrix) * tc_normal[0]); // Scaling must be uniform

	// TODO: Add lights
	//tese_uv_world_space = _global_buffer.view_proj_light * vec4(tese_position_world_space, 1.0f); // fragment uv in clip space

	tese_color = tc_color[0];
	tese_uv = tc_uv[0];

	// TODO: Augment by height map texture
	//vec4 sampled_height = texture(_height_map_texture, texCoords);
	//vec4 position = vec4(p.x, sampled_height.r * yScale - yShift, p.z, 1.0f);

	//tese_height = position.y;

	//vec4 position = vec4(p.x, sin(Rand(vec2(u, v))) * 0.25, p.z, 1.0f);
	//vec4 position = vec4(p.x, p.y, p.z, 1.0f) * sin(Rand(vec2(u, v))) * 0.25;

	float radius = 1.0f;
	vec3 point_on_unit_sphere = normalize(vec3(p.x, p.y, p.z)) * radius;
	vec4 position = vec4(point_on_unit_sphere.xyz, 1.0f);

	// vec4 position = vec4(p.x, p.y, p.z, 1.0f);
	gl_Position = _global_buffer.view_proj * model_matrix * position;
}