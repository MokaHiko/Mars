#version 460

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;
layout(location = 2) in vec3 _normal;
layout(location = 3) in vec2 _uv;

layout(location = 0) out int  v_entity_id;
layout(location = 1) out vec3 v_color;
layout(location = 2) out vec3 v_normal;
layout(location = 3) out vec2 v_uv;

void main()
{
	v_entity_id = gl_BaseInstance;
	v_color = _color;
	v_normal = _normal;
	v_uv = _uv;

	// Pass local space directly
	gl_Position = vec4(_position, 1.0f);
}
