#version 460

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;
layout(location = 2) in vec3 _normal;
layout(location = 3) in vec2 _uv;

layout(location = 0) out vec3 v_position_world_space;
layout(location = 1) out vec3 v_color;
layout(location = 2) out vec2 v_uv;

layout(location = 3) out vec3 v_normal_world_space;
layout(location = 4) out vec4 v_uv_world_space;

struct ObjectData {
  vec4 color;
  mat4 model_matrix;
};

layout(set = 0, binding = 0) uniform GlobalBuffer {
  mat4 view;
  mat4 view_proj;

  vec4 camera_position;
  uint n_dir_lights;
}
_global_buffer;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
  ObjectData s_objects[];
} _object_buffer;

struct Rectangle {
  float x;
  float y;

  float width;
  float height;
};

layout(push_constant) uniform SpritePushConstant { 
	Rectangle atlas_rect;
	Rectangle sprite_rect;
} _sprite_push_constant;

void main() {
  mat4 model_matrix = _object_buffer.s_objects[gl_BaseInstance].model_matrix;

  v_color = _object_buffer.s_objects[gl_BaseInstance].color.xyz;

  Rectangle sprite_rect = _sprite_push_constant.sprite_rect;
  Rectangle atlas_rect = _sprite_push_constant.atlas_rect;

  v_uv = _uv * vec2(sprite_rect.width, sprite_rect.height) / vec2(atlas_rect.width, atlas_rect.height);
  v_uv += vec2(sprite_rect.x / atlas_rect.width, sprite_rect.y / atlas_rect.width);

  v_position_world_space = mat3(model_matrix) * _position;

  v_normal_world_space = normalize(mat3(model_matrix) * _normal); // For uniform scaled objects
  gl_Position = _global_buffer.view_proj * model_matrix * vec4(_position, 1.0f);
}