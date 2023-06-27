#include "ParticleRenderPipeline.h"

#include "Renderer/Renderer.h"

#include <random>
#include <vector>

#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components/Components.h"

#include "ParticleComponents.h"

#include "ToolboX/TimeToolBox.h"
#include "Renderer/Vulkan/VulkanInitializers.h"

// Random engines
static std::uniform_real_distribution<float> rand_dist(0.0f, 1.0f);
static std::default_random_engine rand_engine((unsigned)time(nullptr));

mrs::ParticleRenderPipeline::ParticleRenderPipeline(uint32_t max_particles) 
	: _max_particles(max_particles) {}

mrs::ParticleRenderPipeline::~ParticleRenderPipeline() {}

void mrs::ParticleRenderPipeline::Init() 
{
	_quad_mesh = Mesh::Get("quad");

	uint32_t max_particle_systems = _max_particles / 256;
	_particle_buffer_size = _renderer->PadToStorageBufferSize(sizeof(Particle)) * _max_particles;
	_particle_parameters_size = _renderer->PadToStorageBufferSize(sizeof(ParticleParameters)) * max_particle_systems;
	_buffer_next_free_offset = 0;

	InitComputeDescriptors();
	InitGraphicsDescriptors();

	InitComputePipeline();
	InitGraphicsPipeline();

	UploadResources();
}

void mrs::ParticleRenderPipeline::UploadResources()
{
	// Register each entity particle system
	auto view = _scene->Registry()->view<Transform, ParticleSystem>();
	for (auto entity : view)
	{
		Entity e(entity, _scene);
		auto &particle_system = e.GetComponent<ParticleSystem>();
		RegisterParticleSystem(particle_system);
	}
}

void mrs::ParticleRenderPipeline::InitComputeDescriptors() 
{
	// Create particle buffers
	_particle_storage_buffers.resize(frame_overlaps);
	_particle_parameter_buffers.resize(frame_overlaps);
	for (int i = 0; i < frame_overlaps; i++)
	{
		_particle_storage_buffers[i] = _renderer->CreateBuffer(_particle_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		_particle_parameter_buffers[i] = _renderer->CreateBuffer(_particle_parameters_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	// Create compute descriptor sets
	_compute_descriptor_sets.resize(frame_overlaps);
	for (int i = 0; i < frame_overlaps; i++)
	{
		VkDescriptorBufferInfo parameter_storage_buffer_info = {};
		parameter_storage_buffer_info.buffer = _particle_parameter_buffers[i].buffer;
		parameter_storage_buffer_info.offset = 0;
		parameter_storage_buffer_info.range = _particle_parameters_size;

		VkDescriptorBufferInfo last_frame_storage_buffer_info = {};
		last_frame_storage_buffer_info.buffer = _particle_storage_buffers[(i - 1) % frame_overlaps].buffer;
		last_frame_storage_buffer_info.offset = 0;
		last_frame_storage_buffer_info.range = _particle_buffer_size;

		VkDescriptorBufferInfo current_frame_storage_buffer_info = {};
		current_frame_storage_buffer_info.buffer = _particle_storage_buffers[i].buffer;
		current_frame_storage_buffer_info.offset = 0;
		current_frame_storage_buffer_info.range = _particle_buffer_size;

		vkutil::DescriptorBuilder::Begin(_renderer->_descriptor_layout_cache.get(), _renderer->_descriptor_allocator.get())
			.BindBuffer(0, &parameter_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindBuffer(1, &last_frame_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindBuffer(2, &current_frame_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(&_compute_descriptor_sets[i], &_compute_descriptor_set_layout);
	}

	// Clean up
	_renderer->GetDeletionQueue().Push([&]()
		{
			for (int i = 0; i < _particle_storage_buffers.size(); i++) {
				vmaDestroyBuffer(_renderer->GetAllocator(), _particle_storage_buffers[i].buffer, _particle_storage_buffers[i].allocation);
				vmaDestroyBuffer(_renderer->GetAllocator(), _particle_parameter_buffers[i].buffer, _particle_parameter_buffers[i].allocation);
			} });
}

void mrs::ParticleRenderPipeline::InitComputePipeline() {
  VkPipelineLayoutCreateInfo pipeline_layout_info =
      vkinit::pipeline_layout_create_info();
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &_compute_descriptor_set_layout;

  VkPushConstantRange push_constant = {};
  push_constant.offset = 0;
  push_constant.size = sizeof(ParticleSystemPushConstant);
  push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  pipeline_layout_info.pPushConstantRanges = &push_constant;
  pipeline_layout_info.pushConstantRangeCount = 1;

  VK_CHECK(vkCreatePipelineLayout(_renderer->GetDevice().device,
                                  &pipeline_layout_info, nullptr,
                                  &_compute_pipeline_layout));

  VkComputePipelineCreateInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  info.layout = _compute_pipeline_layout;

  VkShaderModule compute_shader;
  _renderer->LoadShaderModule("assets/shaders/particle_compute_shader.comp.spv",
                              &compute_shader);

  info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  info.stage.pName = "main";
  info.stage.module = compute_shader;

  VK_CHECK(vkCreateComputePipelines(_renderer->GetDevice().device,
                                    VK_NULL_HANDLE, 1, &info, nullptr,
                                    &_compute_pipeline));
}

void mrs::ParticleRenderPipeline::InitGraphicsDescriptors() 
{
	// Graphics descriptor
	_graphics_descriptor_sets.resize(frame_overlaps);
	for (int i = 0; i < frame_overlaps; i++)
	{
		VkDescriptorBufferInfo parameter_storage_buffer_info = {};
		parameter_storage_buffer_info.buffer = _particle_parameter_buffers[i].buffer;
		parameter_storage_buffer_info.offset = 0;
		parameter_storage_buffer_info.range = _particle_parameters_size;

		VkDescriptorBufferInfo particles_storage_buffer_info = {};
		particles_storage_buffer_info.buffer = _particle_storage_buffers[i].buffer;
		particles_storage_buffer_info.offset = 0;
		particles_storage_buffer_info.range = _particle_buffer_size;

		vkutil::DescriptorBuilder::Begin(_renderer->_descriptor_layout_cache.get(), _renderer->_descriptor_allocator.get())
			.BindBuffer(0, &parameter_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.BindBuffer(1, &particles_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(&_graphics_descriptor_sets[i], &_graphics_descriptor_set_layout);
	}
}

void mrs::ParticleRenderPipeline::InitGraphicsPipeline() {
  vkutil::PipelineBuilder builder;

  // Viewport & scissor
  builder._scissor.extent = {_window->GetWidth(), _window->GetHeight()};
  builder._scissor.offset = {0, 0};

  builder._viewport.x = 0.0f;
  builder._viewport.y = 0.0f;
  builder._viewport.width = static_cast<float>(_window->GetWidth());
  builder._viewport.height = static_cast<float>(_window->GetHeight());
  builder._viewport.minDepth = 0.0f;
  builder._viewport.maxDepth = 1.0f;

  // Shader
  VkShaderModule vertex_shader;
  _renderer->LoadShaderModule(
      "assets/shaders/particle_graphics_shader.vert.spv", &vertex_shader);
  VkShaderModule fragment_shader;
  _renderer->LoadShaderModule(
      "assets/shaders/particle_graphics_shader.frag.spv", &fragment_shader);

  builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(
      VK_SHADER_STAGE_VERTEX_BIT, vertex_shader));
  builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(
      VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader));

  // Vertex Input
  VertexInputDescription vertex_desc = Vertex::GetDescription();
  auto bindings = vertex_desc.bindings;
  auto attributes = vertex_desc.attributes;

  builder._vertex_input_info =
      vkinit::pipeline_vertex_input_state_create_info();
  builder._vertex_input_info.vertexBindingDescriptionCount =
      static_cast<uint32_t>(bindings.size());
  builder._vertex_input_info.pVertexBindingDescriptions = bindings.data();
  builder._vertex_input_info.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributes.size());
  builder._vertex_input_info.pVertexAttributeDescriptions = attributes.data();

  builder._input_assembly = vkinit::pipeline_input_assembly_state_create_info(
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  // Graphics Settings
  builder._rasterizer =
      vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL);
  builder._multisampling = vkinit::pipeline_mulitisample_state_create_info();
  builder._color_blend_attachment =
      vkinit::pipeline_color_blend_attachment_state();
  builder._depth_stencil = vkinit::pipeline_depth_stencil_create_info(
      false, false, VK_COMPARE_OP_ALWAYS);

  // Pipeline layouts
  VkPipelineLayoutCreateInfo layout_info =
      vkinit::pipeline_layout_create_info();
  std::vector<VkDescriptorSetLayout> set_layouts = {
      _global_descriptor_set_layout, _object_descriptor_set_layout,
      _default_image_set_layout, _graphics_descriptor_set_layout};

  layout_info.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
  layout_info.pSetLayouts = set_layouts.data();

  VkPushConstantRange push_constant;
  push_constant.offset = 0;
  push_constant.size = sizeof(ParticleSystemPushConstant);
  push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  layout_info.pPushConstantRanges = &push_constant;
  layout_info.pushConstantRangeCount = 1;

  VK_CHECK(vkCreatePipelineLayout(_renderer->GetDevice().device, &layout_info,
                                  nullptr, &_graphics_pipeline_layout));

  builder._pipeline_layout = _graphics_pipeline_layout;

  _graphics_pipeline =
      builder.Build(_renderer->GetDevice().device, _default_render_pass);

  if (_graphics_pipeline == VK_NULL_HANDLE) {
    VK_CHECK(VK_ERROR_UNKNOWN);
  }

  // Clean up
  vkDestroyShaderModule(_renderer->GetDevice().device, vertex_shader, nullptr);
  vkDestroyShaderModule(_renderer->GetDevice().device, fragment_shader,
                        nullptr);
  _renderer->GetDeletionQueue().Push([&]() {
    vkDestroyPipeline(_renderer->GetDevice().device, _graphics_pipeline,
                      nullptr);
    vkDestroyPipelineLayout(_renderer->GetDevice().device,
                            _graphics_pipeline_layout, nullptr);
  });
}

void mrs::ParticleRenderPipeline::Compute(VkCommandBuffer cmd, uint32_t current_frame, float dt)
{
	size_t padded_particle_size = _renderer->PadToStorageBufferSize(sizeof(Particle));
	size_t padded_particle_parameter_size = _renderer->PadToStorageBufferSize(sizeof(ParticleParameters));

	auto view = _scene->Registry()->view<Transform, ParticleSystem>();

	//Update parameters
	char *data;
	vmaMapMemory(_renderer->GetAllocator(), _particle_parameter_buffers[current_frame].allocation, (void **)&data);
	for (auto entity : view)
	{
		Entity e(entity, _scene);
		auto &particle_system = e.GetComponent<ParticleSystem>();

		if (!particle_system.registered)
		{
			RegisterParticleSystem(particle_system);
		}

		if (particle_system.running)
		{
			particle_system.time += dt;
			particle_system.live_particles = glm::min((uint32_t)(particle_system.emission_rate * particle_system.time), particle_system.max_particles);

			// Copy to parameters buffer
			ParticleParameters params = {};
			params.dt = dt;

			params.emission_rate = particle_system.emission_rate;
			params.life_time = particle_system.life_time;
			params.live_particles = particle_system.live_particles;

			params.buffer_offset = particle_system.buffer_offset;
			params.buffer_index = particle_system.buffer_offset / padded_particle_size;

			// Check if reset
			if (particle_system.reset || (!particle_system.repeating && particle_system.time > particle_system.duration))
			{
				// set and reset gpu and cpu flags
				params.reset = 1;
				particle_system.reset = false;
			}

			// Particle system parameters are in a proportional location to particle buffer
			uint32_t offset = (particle_system.buffer_offset / padded_particle_size) * padded_particle_parameter_size;
			memcpy(data + offset, &params, sizeof(ParticleParameters));
		}
	}
	vmaUnmapMemory(_renderer->GetAllocator(), _particle_parameter_buffers[current_frame].allocation);

	// Dispatch per entity particle system 
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline_layout, 0, 1, &_compute_descriptor_sets[current_frame], 0, 0);

	for (auto entity : view)
	{
		Entity e(entity, _scene);
		auto &particle_system = e.GetComponent<ParticleSystem>();

		if (particle_system.running)
		{
			// Bind push constant
			ParticleSystemPushConstant pc;
			pc.count = particle_system.buffer_offset / padded_particle_size;
			vkCmdPushConstants(cmd, _compute_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ParticleSystemPushConstant), &pc);

			// Update particles
			uint32_t x_groups = (particle_system.max_particles / 256) + 1;
			vkCmdDispatch(cmd, x_groups, 1, 1);

			// Check duration reached
			if (!particle_system.repeating && particle_system.time > particle_system.duration)
			{
				particle_system.Stop();
			}
		}
	}

	{
		VkBufferMemoryBarrier compute_graphics_barrier = {};
		compute_graphics_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		compute_graphics_barrier.pNext = nullptr;

		compute_graphics_barrier.buffer = _particle_storage_buffers[current_frame].buffer;
		compute_graphics_barrier.offset = 0;
		compute_graphics_barrier.size = VK_WHOLE_SIZE;

		compute_graphics_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		compute_graphics_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &compute_graphics_barrier, 0, nullptr);
	}
}

void mrs::ParticleRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame)
{
	VkDescriptorSet _frame_object_set = _renderer->GetCurrentGlobalObjectDescriptorSet();

	size_t padded_particle_size = _renderer->PadToStorageBufferSize(sizeof(Particle));
	size_t padded_particle_parameter_size = _renderer->PadToStorageBufferSize(sizeof(ParticleParameters));

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline_layout, 0, 1, &_global_descriptor_set, 0, 0);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline_layout, 1, 1, &_frame_object_set, 0, 0);

	// Bind particle parameters and particles
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline_layout, 3, 1, &_graphics_descriptor_sets[current_frame], 0, 0);

	// Bind quad meshs
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_quad_mesh->_buffer.buffer, &offset);
	vkCmdBindIndexBuffer(cmd, _quad_mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	auto view = _scene->Registry()->view<Transform, ParticleSystem>();

	for (auto entity : view)
	{
		Entity e(entity, _scene);
		auto &particle_system = e.GetComponent<ParticleSystem>();
		auto &material = particle_system.texture;

		// Bind push constant
		ParticleSystemPushConstant pc;
		pc.count = particle_system.buffer_offset / padded_particle_size;
		vkCmdPushConstants(cmd, _graphics_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ParticleSystemPushConstant), &pc);

		// Bind material
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline_layout, 2, 1, &material->texture_set, 0, 0);

		// Draw using instancing
		vkCmdDrawIndexed(cmd, _quad_mesh->_index_count, particle_system.live_particles, 0, 0, e.Id());
	}
}

void mrs::ParticleRenderPipeline::End(VkCommandBuffer cmd) {}

void mrs::ParticleRenderPipeline::RegisterParticleSystem(ParticleSystem &particle_system)
{
	size_t padded_particle_size = _renderer->PadToStorageBufferSize(sizeof(Particle));

	// See if simillair particle system type exists in gpu cache
	auto cache_it = _particle_system_type_cache.find(particle_system);
	if (cache_it != _particle_system_type_cache.end())
	{
		// Check if free instance already in buffer
		if (!cache_it->second.free_buffer_offsets.empty())
		{
			particle_system.buffer_offset = cache_it->second.free_buffer_offsets.back();
			cache_it->second.free_buffer_offsets.pop_back();
		}
		else
		{
			//  Check particle offset is within buffer size
			if (_buffer_next_free_offset / padded_particle_size > _max_particles)
			{
				std::cout << "Max particle count reached!" << std::endl;
				return;
			}

			// Increment buffer
			particle_system.buffer_offset = static_cast<uint32_t>(_buffer_next_free_offset);
			_buffer_next_free_offset += _renderer->PadToStorageBufferSize(sizeof(Particle)) * particle_system.max_particles;

			// Copy particle system data to new offset
			for (uint32_t i = 0; i < frame_overlaps; i++)
			{
				_renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
					{
						VkBufferCopy region = {};
						region.size = cache_it->second.buffer_size;
						region.srcOffset = cache_it->second.first_instance_buffer_offset;
						region.dstOffset = particle_system.buffer_offset;

						vkCmdCopyBuffer(cmd, _particle_storage_buffers[i].buffer, _particle_storage_buffers[i].buffer, 1, &region); });
			}

			// Set reset flag to reset properties
			particle_system.Reset();
		}
	}
	else
	{
		//  Check particle offset is within buffer size
		if (_buffer_next_free_offset / padded_particle_size > _max_particles)
		{
			std::cout << "Max particle count reached!" << std::endl;
			return;
		}

		// Update next buffer offset
		particle_system.buffer_offset = static_cast<uint32_t>(_buffer_next_free_offset);
		_buffer_next_free_offset += padded_particle_size * particle_system.max_particles;

		// Create new particles data
		std::vector<Particle> particles(particle_system.max_particles);

		if (particle_system.emission_shape == EmissionShape::Cone) {
			for (uint32_t i = 0; i < particle_system.max_particles; i++)
			{
				float r = 0.5f * rand_dist(rand_engine);
				float theta = rand_dist(rand_engine) * glm::radians(15.0f);
				float x = r * cos(theta);
				float y = r * sin(theta);
				particles[i].position = glm::vec2(x, y);
				particles[i].velocity = { rand_dist(rand_engine) * particle_system.velocity.x, rand_dist(rand_engine) * particle_system.velocity.y };
				particles[i].color = glm::vec4(particle_system.color, 1.0f);
			}
		}
		else if (particle_system.emission_shape == EmissionShape::Circle) {
			for (uint32_t i = 0; i < particle_system.max_particles; i++)
			{
				float r = 1.0f * rand_dist(rand_engine);
				float theta = rand_dist(rand_engine) * glm::radians(360.0f);
				float x = r * cos(theta);
				float y = r * sin(theta);
				particles[i].position = glm::vec2(x, y);
				particles[i].velocity = rand_dist(rand_engine) * glm::vec2(x, y);
				particles[i].color = glm::vec4(particle_system.color, 1.0f);
			}
		}

		// Cache particle system type
		CacheParticleSystemType(particle_system);

		// Create staging buffer for new particles
		AllocatedBuffer staging_buffer = _renderer->CreateBuffer(padded_particle_size * particle_system.max_particles, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		char *data;
		vmaMapMemory(_renderer->GetAllocator(), staging_buffer.allocation, (void **)&data);
		for (uint32_t i = 0; i < particle_system.max_particles; i++) {
			memcpy(data + (padded_particle_size * i), &particles[i], sizeof(Particle));
		}
		vmaUnmapMemory(_renderer->GetAllocator(), staging_buffer.allocation);

		// Upload particles to offset in global particle buffer
		for (uint32_t i = 0; i < frame_overlaps; i++)
		{
			_renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
				{
					VkBufferCopy region = {};
					region.srcOffset = 0;
					region.size = padded_particle_size * particle_system.max_particles;
					region.dstOffset = particle_system.buffer_offset;

					vkCmdCopyBuffer(cmd, staging_buffer.buffer, _particle_storage_buffers[i].buffer, 1, &region); });
		}

		// Clean up
		vmaDestroyBuffer(_renderer->GetAllocator(), staging_buffer.buffer, staging_buffer.allocation);
	}

	// Signal registered flag
	particle_system.pipeline = this;
	particle_system.registered = true;
}

void mrs::ParticleRenderPipeline::CacheParticleSystemType(ParticleSystem &particle_system)
{
	// Create and cache first instance data
	ParticleSystemType type;
	type.first_instance_buffer_offset = particle_system.buffer_offset;
	type.buffer_size = _renderer->PadToStorageBufferSize(sizeof(Particle)) * particle_system.max_particles;

	_particle_system_type_cache[particle_system] = type;
}

const VkVertexInputBindingDescription &mrs::Particle::GetBinding()
{
	static VkVertexInputBindingDescription binding;

	binding.binding = 0;
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	binding.stride = sizeof(Particle);

	return binding;
}

const std::vector<VkVertexInputAttributeDescription> mrs::Particle::GetAttributes()
{
	static std::vector<VkVertexInputAttributeDescription> attributes;

	if (attributes.size() > 0)
	{
		return attributes;
	}
	attributes.resize(3);

	attributes[0].binding = 0;
	attributes[0].location = 0;
	attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[0].offset = offsetof(Particle, position);

	attributes[1].binding = 0;
	attributes[1].location = 1;
	attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[1].offset = offsetof(Particle, velocity);

	attributes[2].binding = 0;
	attributes[2].location = 2;
	attributes[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributes[2].offset = offsetof(Particle, color);

	return attributes;
}

