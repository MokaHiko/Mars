#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 v_position_world_space;
layout(location = 1) in vec3 v_color;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_normal_world_space;

layout(set = 0, binding = 0) uniform GlobalBuffer{
	mat4 view_proj;
	vec4 direction_light_position;
} _global_buffer;

layout(set = 2, binding = 0) uniform sampler2D _diffuse_texture;

const float AMBIENT = 0.2f;

void main()
{
	//  ~ Diffuse
	vec3 color = texture(_diffuse_texture, v_uv).xyz;

	// ~ Directional
	color *= vec3(max(dot(v_normal_world_space, _global_buffer.direction_light_position.xyz), AMBIENT));

	frag_color = vec4(color, 1.0);
}