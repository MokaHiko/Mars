#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 v_color;
layout(location = 1) in vec2 v_uv;

layout(set = 2, binding = 0) uniform sampler2D _diffuse_texture;

void main()
{
	vec3 color = texture(_diffuse_texture, v_uv).xyz;
	// frag_color = vec4(v_color, 1.0);
	frag_color = vec4(color, 1.0);
}