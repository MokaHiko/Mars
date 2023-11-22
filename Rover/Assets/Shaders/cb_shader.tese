#version 460

layout(quads, equal_spacing, cw) in;

layout(location = 0) in int[] tc_entity_id;
layout(location = 1) in vec3[] tc_color;
layout(location = 2) in vec3[] tc_normal;
layout(location = 3) in vec2[] tc_uv;

layout(location = 0) out vec3 tese_position_world_space;
layout(location = 1) out vec3 tese_color;
layout(location = 2) out vec2 tese_uv;

layout(location = 3) out vec3 tese_normal_world_space;
layout(location = 4) out vec4 tese_uv_world_space;

layout(location = 5) out float tese_height;

struct ObjectData 
{
  vec4 color;
  mat4 model_matrix;
};

layout(set = 0, binding = 0) uniform GlobalBuffer {
  mat4 view;
  mat4 view_proj;

  vec4 camera_position;
  uint n_dir_lights;
} _global_buffer;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
  ObjectData s_objects[];
} _object_buffer;

struct NoiseSetting
{
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

layout(std140, set = 4, binding = 0) readonly buffer NoiseSettingsSSBO{
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
} _cb_data;

//	Simplex 3D Noise
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
float snoise(vec3 v) 
{
  const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

  // First corner
  vec3 i = floor(v + dot(v, C.yyy));
  vec3 x0 = v - i + dot(i, C.xxx);

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  //  x0 = x0 - 0. + 0.0 * C
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

  // Permutations
  i = mod(i, 289.0);
  vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y +
                           vec4(0.0, i1.y, i2.y, 1.0)) +
                   i.x + vec4(0.0, i1.x, i2.x, 1.0));

  // Gradients
  // ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0 / 7.0; // N=7
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_); // mod(j,N)

  vec4 x = x_ * ns.x + ns.yyyy;
  vec4 y = y_ * ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4(x.xy, y.xy);
  vec4 b1 = vec4(x.zw, y.zw);

  vec4 s0 = floor(b0) * 2.0 + 1.0;
  vec4 s1 = floor(b1) * 2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
  vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

  vec3 p0 = vec3(a0.xy, h.x);
  vec3 p1 = vec3(a0.zw, h.y);
  vec3 p2 = vec3(a1.xy, h.z);
  vec3 p3 = vec3(a1.zw, h.w);

  // Normalise gradients
  vec4 norm =
      taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  // Mix final noise value
  vec4 m =
      max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
  m = m * m;
  return 42.0 *
         dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

float yScale = 64.0f;
float yShift = 16.0f;

float EvaluateNoiseFilter(int filter_index, vec3 p)
{
	NoiseSetting noise_filter = noise_settings[filter_index];
  float amplitude = 1.0f;
  float noise_value = 0;
  float frequency = noise_filter.base_roughness;
  for(int i = 0; i < noise_filter.n_layers; i++)
  {
    float v = snoise(p * frequency + noise_filter.center.xyz);
    noise_value += (v + 1) * .5f * amplitude;
    frequency *= noise_filter.roughness;
    amplitude *= noise_filter.persistence;
  }
  noise_value = max(0, noise_value - noise_filter.min_value);
  noise_value *= noise_filter.strength;

  return noise_value;
}

void main() {
  // Weights of the specific patch vertex in the generated quad
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  vec4 p00 = gl_in[0].gl_Position;
  vec4 p10 = gl_in[1].gl_Position;
  vec4 p01 = gl_in[2].gl_Position;
  vec4 p11 = gl_in[3].gl_Position;

  vec2 t00 = tc_uv[0];
  vec2 t10 = tc_uv[1];
  vec2 t01 = tc_uv[2];
  vec2 t11 = tc_uv[3];

  // Interpolate using weights
  vec4 p1 = mix(p00, p10, u);
  vec4 p2 = mix(p01, p11, u);
  vec4 p = mix(p1, p2, v);

  vec2 t0 = mix(t00, t10, u);
  vec2 t1 = mix(t01, t11, u);
  vec2 texCoords = mix(t0, t1, v);

  mat4 model_matrix = _object_buffer.s_objects[tc_entity_id[0]].model_matrix;
  // tese_position_world_space = mat3(model_matrix) * vec3(p); // position in
  // world space
  tese_normal_world_space = normalize(mat3(model_matrix) * tc_normal[0]); // Scaling must be uniform

  tese_color = tc_color[0];
  tese_uv = tc_uv[0];

  float elevation = 0;
  for(int i = 0; i < _cb_data.n_filters; i++)
  {
    elevation += EvaluateNoiseFilter(_cb_data.noise_filters_indices[i], p.xyz);
  }

  float radius = 1;
  vec3 point_on_unit_sphere = normalize(vec3(p.x, p.y, p.z)) * radius * (1 + elevation);
  vec4 position = vec4(point_on_unit_sphere.xyz, 1.0f);
  tese_position_world_space = mat3(model_matrix) * vec3(position); // position in world space

  tese_height = elevation;

  gl_Position = _global_buffer.view_proj * model_matrix * position;
}