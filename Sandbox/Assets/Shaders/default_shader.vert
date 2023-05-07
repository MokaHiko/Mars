#version 460

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;

layout(location = 0) out vec3 v_color;

struct StaticObjectData {
	vec4 color;
	mat4 model_matrix;
};

layout(std140, set = 0, binding = 0) readonly buffer GlobalData {
	StaticObjectData s_objects[];
} _global_data;

void main()
{
	v_color = _global_data.s_objects[gl_BaseInstance].color.xyz;
	gl_Position = _global_data.s_objects[gl_BaseInstance].model_matrix * vec4(_position, 1.0f);
}