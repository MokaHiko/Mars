#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 v_position_world_space;
layout(location = 1) in vec3 v_color;
layout(location = 2) in vec2 v_uv;

layout(location = 3) in vec3 v_normal_world_space;
layout(location = 4) in vec4 v_uv_world_space;

struct DirectionalLight
{
	mat4 view_proj;

	vec4 Direction;
	vec4 Ambient;
	vec4 Diffuse;
	vec4 Specular;
};

layout(set = 0, binding = 0) uniform GlobalBuffer {
	mat4 view;
	mat4 view_proj;

	vec4 camera_position;
	uint n_dir_lights;
} _global_buffer;

layout(set = 2, binding = 0) uniform Material {
	// Albedo
	vec4 diffuse_color;

	// Metallic
	float metallic;
	float specular;
	int texture_channel;

	bool receive_shadows;
} _material;
layout(set = 2, binding = 1) uniform sampler2D _diffuse_texture;
layout(set = 2, binding = 2) uniform sampler2D _specular_texture;


void main()
{
	vec3 final_color = {0.0, 0.0, 0.0};

	vec3 normal = v_normal_world_space;
	vec3 view_dir = normalize(_global_buffer.camera_position.xyz - v_position_world_space);

	final_color = vec3(1.0f, 1.0f, 1.0f);
	frag_color = vec4(final_color, 1);
}