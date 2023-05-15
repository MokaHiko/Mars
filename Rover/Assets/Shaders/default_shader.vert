#version 460

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;
layout(location = 2) in vec3 _normal;
layout(location = 3) in vec2 _uv;

layout(location = 0) out vec3 v_position_world_space;
layout(location = 1) out vec3 v_color;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec3 v_normal_world_space;

struct ObjectData {
	vec4 color;
	mat4 model_matrix;
};

layout(set = 0, binding = 0) uniform GlobalBuffer{
	mat4 view_proj;
	vec4 direction_light_position;
} _global_buffer;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer{
	ObjectData s_objects[];
} _object_buffer;

void main()
{
	mat4 model_matrix = _object_buffer.s_objects[gl_BaseInstance].model_matrix;

	// Scaling must be uniform
	v_position_world_space = mat3(model_matrix) * _position;
	v_color = _color;
	v_normal_world_space = normalize(mat3(model_matrix) * _normal);
	v_uv = _uv;

	gl_Position = _global_buffer.view_proj * model_matrix * vec4(_position, 1.0f);
}