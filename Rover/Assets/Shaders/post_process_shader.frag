#version 460

layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 0) uniform sampler2D _screen_texture;

layout(location = 3) in vec2 v_uv;

void main() 
{
	vec4 color = texture(_screen_texture, v_uv);
	
	// Remove red
	// float grey = dot( color.rgb, vec3( 0.2, 0.7, 0.1 ) );
	// frag_color = grey * vec4( 1.5, 1.0, 0.5, 1.0 );

	frag_color = color;
}