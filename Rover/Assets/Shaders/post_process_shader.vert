#version 460

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;
layout(location = 2) in vec3 _normal;
layout(location = 3) in vec2 _uv;

layout(location = 3) out vec2 v_uv;

void main()
{
	v_uv = vec2(_uv.x, -_uv.y);
	gl_Position =  vec4(_position, 1.0f);
}