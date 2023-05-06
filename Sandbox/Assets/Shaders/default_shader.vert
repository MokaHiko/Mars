#version 450

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;

layout(location = 0) out vec3 v_color;

void main()
{
	v_color = _color;
	gl_Position = vec4(_position, 1.0f);
}