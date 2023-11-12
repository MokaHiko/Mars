#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 tese_position_world_space;
layout(location = 1) in vec3 tese_color;
layout(location = 2) in vec2 tese_uv;

layout(location = 3) in vec3 tese_normal_world_space;
layout(location = 4) in vec4 tese_uv_world_space;

// layout(location = 5) in float tese_height;

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

layout(std140, set = 3, binding = 0) readonly buffer DirLights{
    DirectionalLight dir_lights[];
};

// layout(set = 3, binding = 0) uniform sampler2D _shadow_map_texture;

// float CalculateShadowFactor()
// {
// 	// ~ Shadow shadow
// 	float shadow_factor = 1.0f;
// 	vec3 shadow_coords = tese_uv_world_space.xyz / tese_uv_world_space.w; // to turn coords into screen space manually

// 	// Remap to [0.0 to 1.0] 
// 	shadow_coords.xy = shadow_coords.xy * 0.5 + 0.5;

// 	// Comparing shadow_map z is closer than current frag z
// 	float bias = 0.005f;
// 	float shadow_map_value = texture(_shadow_map_texture, shadow_coords.xy).r;
// 	if(texture(_shadow_map_texture, shadow_coords.xy).r < shadow_coords.z - bias) {
// 		shadow_factor = 0.5f;
// 	}

// 	return shadow_factor;
// }

void main()
{
	// ~ Shadow shadow
	//float shadow_factor = CalculateShadowFactor();
	// float shadow_factor = 1;

	//  ~ Diffuse
	vec3 color = texture(_diffuse_texture, tese_uv).xyz * _material.diffuse_color.xyz;

	// ~ Directional
	//float diff = max(dot(tese_normal_world_space, normalize(_global_buffer.direction_light_position.xyz)), 0);
	//color *= diff;
	// frag_color = shadow_factor * vec4(color, 1);

	//float h = (tese_height + 16) / 64.0f;

	frag_color = vec4(1.0, 1.0, 1.0, 1.0);
}

// #version 460

// layout(location = 0) out vec4 frag_color;

// layout(location = 0) in vec3 v_position_world_space;
// layout(location = 1) in vec3 v_color;
// layout(location = 2) in vec2 v_uv;

// layout(location = 3) in vec3 v_normal_world_space;
// layout(location = 4) in vec4 v_uv_world_space;

// struct DirectionalLight
// {
// 	mat4 view_proj;

// 	vec4 Direction;
// 	vec4 Ambient;
// 	vec4 Diffuse;
// 	vec4 Specular;
// };

// layout(set = 0, binding = 0) uniform GlobalBuffer {
// 	mat4 view;
// 	mat4 view_proj;

// 	vec4 camera_position;
// 	uint n_dir_lights;
// } _global_buffer;

// layout(set = 2, binding = 0) uniform Material {
// 	// Albedo
// 	vec4 diffuse_color;

// 	// Metallic
// 	float metallic;
// 	float specular;
// 	int texture_channel;

// 	bool receive_shadows;
// } _material;
// layout(set = 2, binding = 1) uniform sampler2D _diffuse_texture;
// layout(set = 2, binding = 2) uniform sampler2D _specular_texture;

// // TODO: Celstial Shadows
// // layout(set = 3, binding = 0) uniform sampler2D _shadow_map_texture;

// layout(std140, set = 3, binding = 0) readonly buffer DirLights{
//     DirectionalLight dir_lights[];
// };

// float CalculateShadowFactor()
// {
// 	// ~ Shadow shadow
// 	float shadow_factor = 1.0f;
// 	vec3 shadow_coords = v_uv_world_space.xyz / v_uv_world_space.w; // to turn coords into screen space manually

// 	// Normalize between [0.0 to 1.0] 
// 	shadow_coords.xy = shadow_coords.xy * 0.5 + 0.5;

// 	// Comparing shadow_map z is closer than current frag z
// 	// float bias = 0.005f;
// 	// float shadow_map_value = texture(_shadow_map_texture, shadow_coords.xy).r;

// 	// if(texture(_shadow_map_texture, shadow_coords.xy).r < shadow_coords.z - bias) 
// 	// {
// 	// 	shadow_factor = 0.5f;
// 	// }

// 	return shadow_factor;
// }

// vec3 CalculateDirLight(DirectionalLight light, vec3 normal, vec3 view_dir)
// {
// 	float diffuse_factor = max(dot(normal, -light.Direction.xyz), 0.0);

// 	vec3 reflect_dir = reflect(-light.Direction.xyz, normal);
// 	float specular_factor = pow(max(dot(view_dir, reflect_dir), 0.0f), 32);

// 	vec3 ambient = light.Ambient.xyz * texture(_diffuse_texture, v_uv).xyz * _material.diffuse_color.xyz;
// 	vec3 diffuse = light.Diffuse.xyz * texture(_diffuse_texture, v_uv).xyz * diffuse_factor;
// 	vec3 specular = light.Specular.xyz * texture(_specular_texture, v_uv).xyz * specular_factor;

// 	return ambient + diffuse + specular;
// }

// void main()
// {
// 	vec3 final_color = {0.0, 0.0, 0.0};

// 	vec3 normal = v_normal_world_space;
// 	vec3 view_dir = normalize(_global_buffer.camera_position.xyz - v_position_world_space);

// 	// ~ Directional Lights
// 	for(uint i = 0; i < _global_buffer.n_dir_lights; i++)
// 	{
// 		final_color += CalculateDirLight(dir_lights[i], normal, view_dir);
// 	}

// 	// TODO: Celstial Shadows
// 	// ~ Shadow shadow
// 	//float shadow_factor = CalculateShadowFactor();
// 	//frag_color = shadow_factor * vec4(final_color, 1);

// 	frag_color = vec4(final_color, 1);
// }