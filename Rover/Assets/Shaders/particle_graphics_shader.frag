#version 460
layout(location = 0) out vec4 frag_color;

layout(location = 1) in vec4 v_color;

layout(location = 3) in vec2 v_uv;

struct MaterialData {
	// Albedo
	vec4 diffuse_color;

	// Metallic
	float metallic;
	float specular;
	int texture_channel;

	bool receive_shadows;
};

layout(std140, set = 2, binding = 0) readonly buffer Materials{
	MaterialData materials[];
} _materials_buffer;

layout(set = 2, binding = 1) uniform sampler2D _diffuse_texture;

layout( push_constant ) uniform ParticleSystemPushConstant{
	uint count;     // index into particle system buffer
    uint material_index;  // index into global materials buffer
} _particle_push_constant;

void main()
{
	// Get material in material buffer
	MaterialData material = _materials_buffer.materials[_particle_push_constant.material_index];

    vec4 diffuse = texture(_diffuse_texture, v_uv);

    if(diffuse.a <= 0.1f)  
    {
        discard;
    }

    frag_color = diffuse * v_color;
}