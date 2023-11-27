#version 460
layout(location = 0) out vec4 frag_color;

layout(location = 1) in vec4 v_color;

layout(location = 3) in vec2 v_uv;

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
    vec4 diffuse = texture(_diffuse_texture, v_uv);

    // if(diffuse.a <= 0.01f)  
    // {
    //     discard;
    // }

    frag_color = diffuse * v_color;
}