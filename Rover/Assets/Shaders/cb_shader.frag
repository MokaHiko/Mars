#version 460

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec3 tese_position_world_space;
layout(location = 1) in vec3 tese_color;
layout(location = 2) in vec2 tese_uv;

layout(location = 3) in vec3 tese_normal_world_space;
layout(location = 4) in vec4 tese_uv_world_space;
layout(location = 5) in float tese_height;

struct DirectionalLight {
  mat4 view_proj;

  vec4 Direction;
  vec4 Ambient;
  vec4 Diffuse;
  vec4 Specular;
};

layout(set = 0, binding = 0) uniform GlobalBuffer {
  mat4 view;
  mat4 view_proj;

  vec4 camera_position;
  uint n_dir_lights;
}
_global_buffer;

layout(set = 2, binding = 0) uniform Material {
  // Albedo
  vec4 diffuse_color;

  // Metallic
  float metallic;
  float specular;
  int texture_channel;

  bool receive_shadows;
}
_material;
layout(set = 2, binding = 1) uniform sampler2D _diffuse_texture;
layout(set = 2, binding = 2) uniform sampler2D _specular_texture;

layout(std140, set = 3, binding = 0) readonly buffer DirLights {
  DirectionalLight dir_lights[];
};

struct NoiseSetting {
  uint n_layers;
  float persistence;

  float base_roughness;
  float roughness;

  uint min_resolution;
  uint max_resolution;

  float strength;
  float min_value;

  vec4 center;
};

layout(std140, set = 4, binding = 0) readonly buffer NoiseSettingsSSBO {
  NoiseSetting noise_settings[];
};

layout(push_constant) uniform CelestialBodyData {
  float dt;
  float mask;
  uint n_filters;
  uint type;

  ivec4 noise_filters_indices;

  // Gradients
  vec4 color_1;
  vec4 color_2;
  vec4 color_3;
  vec4 color_4;
}
cb_data;

vec3 CalculateDirLight(DirectionalLight light, vec3 normal, vec3 view_dir) {
  float diffuse_factor = max(dot(normal, -light.Direction.xyz), 0.0);

  vec3 reflect_dir = reflect(-light.Direction.xyz, normal);
  float specular_factor = pow(max(dot(view_dir, reflect_dir), 0.0f), 32);

  vec3 ambient = light.Ambient.xyz * texture(_diffuse_texture, tese_uv).xyz *
                 _material.diffuse_color.xyz;
  vec3 diffuse = light.Diffuse.xyz * texture(_diffuse_texture, tese_uv).xyz *
                 diffuse_factor;
  vec3 specular = light.Specular.xyz * texture(_specular_texture, tese_uv).xyz *
                  specular_factor;

  return ambient + diffuse + specular;
}

void main() {
  vec3 final_color = {0.0, 0.0, 0.0};

  vec3 normal = tese_normal_world_space;
  vec3 view_dir =
      normalize(_global_buffer.camera_position.xyz - tese_position_world_space);

  // ~ Directional Lights
  for (uint i = 0; i < _global_buffer.n_dir_lights; i++) {
    final_color += CalculateDirLight(dir_lights[i], normal, view_dir);
  }

  // ~ Shadow shadow
  // float shadow_factor = CalculateShadowFactor();
  // frag_color = shadow_factor * vec4(final_color, 1);

  float step1 = cb_data.color_1.w;
  float step2 = cb_data.color_2.w;;
  float step3 = cb_data.color_3.w;
  float step4 = cb_data.color_4.w;

  // Gradient
  vec4 color = mix(cb_data.color_1, cb_data.color_2, smoothstep(step1, step2, tese_height));
  color = mix(color, cb_data.color_3, smoothstep(step2, step3, tese_height));
  color = mix(color, cb_data.color_4, smoothstep(step3, step4, tese_height));

  final_color *= color.xyz;

  frag_color = vec4(final_color, 1);
}