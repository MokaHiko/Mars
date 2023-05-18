#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 v_position_world_space;
layout(location = 1) in vec3 v_color;
layout(location = 2) in vec2 v_uv;

layout(location = 3) in vec3 v_normal_world_space;
layout(location = 4) in vec4 v_uv_world_space;

layout(set = 0, binding = 0) uniform GlobalBuffer{
	mat4 view_proj;
	mat4 view_proj_camera;
	vec4 direction_light_position;
} _global_buffer;

layout(set = 2, binding = 0) uniform sampler2D _diffuse_texture;
layout(set = 3, binding = 0) uniform sampler2D _shadow_map_texture;

const float AMBIENT = 0.01f;

float near = 0.1f; 
float far  = 1000.0f; 

void main()
{
	// ~ Shadow shadow
	float shadow_factor = 1.0f;
	vec4 shadow_coords = v_uv_world_space / v_uv_world_space.w; // to turn coords into screen space manually

	// Comparing shadow_map z is closer than current frag z
	float bias = 0.0005f;
	if(texture(_shadow_map_texture, shadow_coords.xy).r < shadow_coords.z - bias) {
		shadow_factor = 0.15f;
	}
	//  ~ Diffuse
	vec3 color = texture(_diffuse_texture, v_uv).xyz;

	// ~ Directional
	color *= vec3(max(dot(v_normal_world_space, _global_buffer.direction_light_position.xyz), 0));
	frag_color = shadow_factor * vec4(color, 1);
}