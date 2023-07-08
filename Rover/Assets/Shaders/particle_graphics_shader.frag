#version 460
layout(location = 0) out vec4 frag_color;

layout(location = 1) in vec4 v_color;

layout(location = 3) in vec2 v_uv;

layout(set = 2, binding = 0) uniform sampler2D _diffuse_texture;

void main()
{
    vec4 diffuse = texture(_diffuse_texture, v_uv);

    if(diffuse.a <= 0.1f)  
    {
        discard;
    }

    frag_color = diffuse * v_color;
}