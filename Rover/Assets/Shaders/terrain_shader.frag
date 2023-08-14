#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 tese_position_world_space;
layout(location = 1) in vec3 tese_color;
layout(location = 2) in vec2 tese_uv;

layout(location = 3) in vec3 tese_normal_world_space;
layout(location = 4) in vec4 tese_uv_world_space;

layout(location = 5) in float tese_height;

struct MaterialData {
	// Albedo
	vec4 diffuse_color;

	// Metallic
	float metallic;
	float specular;
	int texture_channel;

	bool receive_shadows;
};

layout(set = 0, binding = 0) uniform GlobalBuffer{
	mat4 view;
	mat4 view_proj;
	mat4 view_proj_light;
	vec4 direction_light_position;
} _global_buffer;

layout(std140, set = 2, binding = 0) readonly buffer Materials {
	MaterialData materials[];
} _materials_buffer;
layout(set = 2, binding = 1) uniform sampler2D _diffuse_texture;

layout(set = 3, binding = 0) uniform sampler2D _shadow_map_texture;

layout( push_constant ) uniform constants {
    uint material_index; 
} _material_index;

float CalculateShadowFactor()
{
	// ~ Shadow shadow
	float shadow_factor = 1.0f;
	vec3 shadow_coords = tese_uv_world_space.xyz / tese_uv_world_space.w; // to turn coords into screen space manually

	// Remap to [0.0 to 1.0] 
	shadow_coords.xy = shadow_coords.xy * 0.5 + 0.5;

	// Comparing shadow_map z is closer than current frag z
	float bias = 0.005f;
	float shadow_map_value = texture(_shadow_map_texture, shadow_coords.xy).r;
	if(texture(_shadow_map_texture, shadow_coords.xy).r < shadow_coords.z - bias) {
		shadow_factor = 0.5f;
	}

	return shadow_factor;
}

void main()
{
	// Get material in material buffer
	MaterialData material = _materials_buffer.materials[_material_index.material_index];

	// ~ Shadow shadow
	//float shadow_factor = CalculateShadowFactor();
	float shadow_factor = 1;

	//  ~ Diffuse
	vec3 color = texture(_diffuse_texture, tese_uv).xyz * material.diffuse_color.xyz;

	// ~ Directional
	float diff = max(dot(tese_normal_world_space, normalize(_global_buffer.direction_light_position.xyz)), 0);
	color *= diff;
	// frag_color = shadow_factor * vec4(color, 1);

	float h = (tese_height + 16) / 64.0f;
	frag_color = vec4(h, h, h, 1.0);
}