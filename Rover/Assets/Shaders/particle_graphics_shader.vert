#version 460

layout(location = 1) out vec4 v_color;
layout(location = 3) out vec2 v_uv;

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _color;
layout(location = 2) in vec3 _normal;
layout(location = 3) in vec2 _uv;

struct ObjectData {
	vec4 color;
	mat4 model_matrix;
};

struct Particle
{
    vec2 position;
    vec2 velocity;
    vec4 color;

    float time_elapsed;
};

struct ParticleParameters 
{
    // Color Gradient
    vec4 color_1;
    vec4 color_2;

    // Shape
    float scale;

    // Emission
    float life_time; // particles lifetiem in seconds;
    float dt; // delta time this frame in seconds
    float emission_rate;
    uint live_particles;
    uint reset; // set to 1 if reset

    // Buffer offsets
    uint buffer_offset; // Offset into global particle buffer
    uint buffer_index; // Offset into global particle buffer array
};

layout(set = 0, binding = 0) uniform GlobalBuffer{
	mat4 view_proj;
	mat4 view_proj_light;
	vec4 direction_light_position;
} _global_buffer;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer{
	ObjectData s_objects[];
} _object_buffer;

layout(std140, set = 3, binding = 0) readonly buffer ParticleParametersUBO {
    ParticleParameters[] parameters;
} _particle_parameters_array;

layout(std140, set = 3, binding = 1) readonly buffer Particles{
    Particle particles[];
} _particles;

//push constants block
layout( push_constant ) uniform constants {
    uint count; // the nth particle system this frame
} p_particle_system_count;

void main()
{
    const mat4 view_proj = _global_buffer.view_proj;

    ParticleParameters particle_parameters = _particle_parameters_array.parameters[p_particle_system_count.count];
    uint global_particle_index = (gl_InstanceIndex - gl_BaseInstance) + particle_parameters.buffer_index;

    mat4 model_matrix = _object_buffer.s_objects[gl_BaseInstance].model_matrix;

    model_matrix[0][0] *= particle_parameters.scale;
    model_matrix[1][1] *= particle_parameters.scale;
    model_matrix[2][2] *= particle_parameters.scale;

    Particle particle = _particles.particles[global_particle_index];
    v_color = particle.color;
    v_uv = _uv;

    vec4 pos = vec4(_position + vec3(particle.position, 0.0f), 1.0f);
    gl_Position = view_proj * model_matrix * pos;
}