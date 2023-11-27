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

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

mrs::ParticleRenderPipeline::ParticleRenderPipeline(uint32_t max_particles)
	: IRenderPipeline("ParticleRenderPipeline"), _max_particles(max_particles) {}

mrs::ParticleRenderPipeline::~ParticleRenderPipeline() {}

void mrs::ParticleRenderPipeline::Init()
{
	Scene* scene = Application::Instance().GetScene();
	scene->Registry()->on_construct<ParticleSystem>().connect<&ParticleRenderPipeline::OnParticleSystemCreated>(this);
	scene->Registry()->on_destroy<ParticleSystem>().connect<&ParticleRenderPipeline::OnParticleSystemDestroyed>(this);

	uint32_t max_particle_systems = _max_particles / 256;
	_buffer_next_free_offset = 0;

	// Pre calculate buffer sizes
	_padded_particle_size = _renderer->PadToStorageBufferSize(sizeof(Particle));
	_padded_particle_parameter_size = _renderer->PadToStorageBufferSize(sizeof(ParticleParameters));
	_particle_buffer_size = _padded_particle_size * _max_particles;
	_particle_parameters_size = _padded_particle_parameter_size * max_particle_systems;

	InitComputeSyncStructures();

	InitComputeDescriptors();
	InitGraphicsDescriptors();

	InitComputePipeline();
	InitGraphicsPipeline();

	UploadResources();

	// Define Effect Template
	std::vector<ShaderEffect*> default_particle_effects;
	default_particle_effects.push_back(Effect().get());
	Ref<EffectTemplate> default_particle = VulkanAssetManager::Instance().CreateEffectTemplate(default_particle_effects, "default_particle");
	Material::Create(default_particle, Texture::Get("default"), "default_particle");
}

void mrs::ParticleRenderPipeline::UploadResources()
{
	// Register each entity particle system
	auto scene = Application::Instance().GetScene();
	auto view = scene->Registry()->view<Transform, ParticleSystem>();
	for (auto entity : view)
	{
		Entity e(entity, scene);
		auto& particle_system = e.GetComponent<ParticleSystem>();

		if (!particle_system.registered)
		{
			RegisterParticleSystem(particle_system);
		}
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

		vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
			.BindBuffer(0, &parameter_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindBuffer(1, &last_frame_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindBuffer(2, &current_frame_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(&_compute_descriptor_sets[i], &_compute_descriptor_set_layout);
	}

	// Clean up
	_renderer->DeletionQueue().Push([&]()
		{
			for (int i = 0; i < _particle_storage_buffers.size(); i++)
			{
				vmaDestroyBuffer(_renderer->Allocator(), _particle_storage_buffers[i].buffer, _particle_storage_buffers[i].allocation);
				vmaDestroyBuffer(_renderer->Allocator(), _particle_parameter_buffers[i].buffer, _particle_parameter_buffers[i].allocation);
			}
		});
}

void mrs::ParticleRenderPipeline::InitComputePipeline() {
	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &_compute_descriptor_set_layout;

	VkPushConstantRange push_constant = {};
	push_constant.offset = 0;
	push_constant.size = sizeof(ParticleSystemPushConstant);
	push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	pipeline_layout_info.pPushConstantRanges = &push_constant;
	pipeline_layout_info.pushConstantRangeCount = 1;

	VK_CHECK(vkCreatePipelineLayout(_renderer->Device().device,
		&pipeline_layout_info, nullptr,
		&_compute_pipeline_layout));

	VkComputePipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	info.layout = _compute_pipeline_layout;

	VkShaderModule compute_shader;
	VulkanAssetManager::Instance().LoadShaderModule("assets/shaders/particle_compute_shader.comp.spv", &compute_shader);

	info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	info.stage.pName = "main";
	info.stage.module = compute_shader;

	VK_CHECK(vkCreateComputePipelines(_renderer->Device().device,
		VK_NULL_HANDLE, 1, &info, nullptr,
		&_compute_pipeline));
}

void mrs::ParticleRenderPipeline::InitComputeSyncStructures()
{
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	_compute_in_flight_fences.resize(frame_overlaps);
	_compute_in_flight_semaphores.resize(frame_overlaps);
	for (uint32_t i = 0; i < frame_overlaps; i++)
	{
		VK_CHECK(vkCreateSemaphore(_renderer->Device().device, &semaphore_info, nullptr, &_compute_in_flight_semaphores[i]));
		VK_CHECK(vkCreateFence(_renderer->Device().device, &fence_info, nullptr, &_compute_in_flight_fences[i]));

		_renderer->DeletionQueue().Push([&]()
			{
				vkDestroyFence(_renderer->Device().device, _compute_in_flight_fences[i], nullptr);
				vkDestroySemaphore(_renderer->Device().device, _compute_in_flight_semaphores[i], nullptr);
			});
	}
}

void mrs::ParticleRenderPipeline::UpdateComputeDescriptorSets(uint32_t current_frame, float dt, RenderableBatch* batch)
{
	// Update parameters
	char* data;
	vmaMapMemory(_renderer->Allocator(), _particle_parameter_buffers[current_frame].allocation, (void**)&data);
	int ctr = 0;
	for (auto& e : batch->entities) {
		auto& particle_system = e.GetComponent<ParticleSystem>();

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
			params.color_1 = particle_system.color_1;
			params.color_2 = particle_system.color_2;

			params.scale = particle_system.particle_size;

			params.emission_rate = particle_system.emission_rate;
			params.life_time = particle_system.life_time;
			params.live_particles = particle_system.live_particles;
			params.dt = dt;

			params.buffer_offset = particle_system.buffer_offset;
			params.buffer_index = particle_system.buffer_offset / static_cast<uint32_t>(_padded_particle_size);

			// Check if reset
			if (particle_system.reset || (!particle_system.repeating && particle_system.time > particle_system.duration))
			{
				// set and reset gpu and cpu flags
				params.reset = 1;
				particle_system.reset = false;
				particle_system.live_particles = 0;
				particle_system.time = 0.0f;
			}

			if (particle_system.stop)
			{
				params.reset = 1;
				particle_system.reset = false;
			}

			// Particle system parameters are in a proportional location to particle buffer
			size_t offset = ctr * _padded_particle_parameter_size;
			ctr++;

			memcpy(data + offset, &params, sizeof(ParticleParameters));

			// Stop is handled after reset
			if (particle_system.stop)
			{
				particle_system.running = false;
				particle_system.stop = false;

				particle_system.live_particles = 0;
				particle_system.time = 0.0f;
			}
		}
	}
	vmaUnmapMemory(_renderer->Allocator(), _particle_parameter_buffers[current_frame].allocation);
}

void mrs::ParticleRenderPipeline::RecordComputeCommandBuffers(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
	VkCommandBufferBeginInfo beginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

	// Dispatch per entity particle system
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline_layout, 0, 1, &_compute_descriptor_sets[current_frame], 0, 0);

	int ctr = 0;
	for (auto e : batch->entities) {
		auto& particle_system = e.GetComponent<ParticleSystem>();

		if (particle_system.running)
		{
			// Bind push constant
			ParticleSystemPushConstant pc;
			pc.index = ctr++;

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
		// COMPUTE RELEASE BARRIERS: Particle Systems Storage Buffer Memory Barriers
		VkBufferMemoryBarrier particle_parameter_barrier = vkinit::BufferMemoryBarrier(
			_particle_parameter_buffers[current_frame].buffer,
			_particle_parameters_size,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			_renderer->GetQueueIndices().compute,
			_renderer->GetQueueIndices().graphics
		);

		VkBufferMemoryBarrier particle_barrier = vkinit::BufferMemoryBarrier(
			_particle_storage_buffers[current_frame].buffer,
			_particle_buffer_size,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			_renderer->GetQueueIndices().compute,
			_renderer->GetQueueIndices().graphics
		);

		VkBufferMemoryBarrier barriers[] = { particle_parameter_barrier, particle_barrier };
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 2, barriers, 0, nullptr);
	}

	VK_CHECK(vkEndCommandBuffer(cmd));
}

void mrs::ParticleRenderPipeline::FillParticleArray(const ParticleSystem& particle_system, std::vector<Particle>& particles)
{
	particles.resize(particle_system.max_particles);

	if (particle_system.emission_shape == EmissionShape::Cone) {
		for (uint32_t i = 0; i < particle_system.max_particles; i++)
		{
			float r = 0.5f * _random_generator.Next();
			float theta = (3.14159265f / 2.0f) + (glm::radians(particle_system.spread_angle) * 0.5f * _random_generator_negative_to_one.Next());
			float x = r * cos(theta);
			float y = r * sin(theta);
			particles[i].position = glm::vec2(x, y);
			particles[i].velocity = particles[i].position * particle_system.velocity.x;
			particles[i].color = particle_system.color_1;
		}
	}
	else if (particle_system.emission_shape == EmissionShape::Circle)
	{
		for (uint32_t i = 0; i < particle_system.max_particles; i++)
		{
			float r = 0.5f * _random_generator.Next();
			float theta = _random_generator.Next() * glm::radians(360.0f);
			float x = r * cos(theta);
			float y = r * sin(theta);
			particles[i].position = glm::vec2(x, y);
			particles[i].velocity = particles[i].position * (1.0f + (_random_generator.Next() * particle_system.velocity));
			particles[i].color = particle_system.color_1;
		}
	}
}

void mrs::ParticleRenderPipeline::InitGraphicsDescriptors()
{
	VkDescriptorBufferInfo global_buffer_info = {};
	global_buffer_info.buffer = _renderer->GlobalBuffer().buffer;
	global_buffer_info.offset = 0;
	global_buffer_info.range = VK_WHOLE_SIZE;

	vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
		.BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(&_global_data_set, &_global_data_set_layout);

	_object_sets.resize(frame_overlaps);
	_dir_light_sets.resize(frame_overlaps);
	for (uint32_t i = 0; i < frame_overlaps; i++)
	{
		VkDescriptorBufferInfo global_buffer_info = {};
		global_buffer_info.buffer = _renderer->ObjectBuffers()[i].buffer;
		global_buffer_info.offset = 0;
		global_buffer_info.range = VK_WHOLE_SIZE;

		vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
			.BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build(&_object_sets[i], &_object_set_layout);

		VkDescriptorBufferInfo dir_light_buffer_info = {};
		dir_light_buffer_info.buffer = _renderer->DirLightBuffers()[i].buffer;
		dir_light_buffer_info.offset = 0;
		dir_light_buffer_info.range = VK_WHOLE_SIZE;

		vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
			.BindBuffer(0, &dir_light_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(&_dir_light_sets[i], &_dir_light_layout);
	}

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

		vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
			.BindBuffer(0, &parameter_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.BindBuffer(1, &particles_storage_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build(&_graphics_descriptor_sets[i], &_graphics_descriptor_set_layout);
	}
}

void mrs::ParticleRenderPipeline::InitGraphicsPipeline() {
	Ref<Shader> particle_vertex_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/particle_graphics_shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	PushShader(particle_vertex_shader);

	Ref<Shader> particle_fragment_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/particle_graphics_shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	PushShader(particle_fragment_shader);

	VkPushConstantRange push_constant = {};
	push_constant.offset = 0;
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constant.size = sizeof(ParticleSystemPushConstant);
	_render_pipeline_settings.push_constants.push_back(push_constant);

	_render_pipeline_settings.blend_enable = VK_TRUE;
	_render_pipeline_settings.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	_render_pipeline_settings.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	_render_pipeline_settings.colorBlendOp = VK_BLEND_OP_ADD;
	_render_pipeline_settings.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	_render_pipeline_settings.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	_render_pipeline_settings.alphaBlendOp = VK_BLEND_OP_ADD;

	_render_pass = _renderer->_offscreen_render_pass;
	BuildPipeline();
}

void mrs::ParticleRenderPipeline::Compute(VkCommandBuffer cmd, uint32_t current_frame, float dt, RenderableBatch* compute_batch)
{
	VkDevice device = _renderer->Device().device;

	vkWaitForFences(device, 1, &_compute_in_flight_fences[current_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	UpdateComputeDescriptorSets(current_frame, dt, compute_batch);
	vkResetFences(device, 1, &_compute_in_flight_fences[current_frame]);

	vkResetCommandBuffer(cmd, 0);
	RecordComputeCommandBuffers(cmd, current_frame, compute_batch);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &_compute_in_flight_semaphores[current_frame];

	_renderer->PushGraphicsSemaphore(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, _compute_in_flight_semaphores[current_frame]);
	VK_CHECK(vkQueueSubmit(_renderer->Queues().compute, 1, &submit_info, _compute_in_flight_fences[current_frame]));
}


void mrs::ParticleRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
	static Ref<Mesh> quad_mesh = Mesh::Get("quad");

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_set, 0, 0);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[current_frame], 0, 0);

	// Bind particle parameters and particles
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 3, 1, &_graphics_descriptor_sets[current_frame], 0, 0);

	// Bind quad meshs
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &quad_mesh->_buffer.buffer, &offset);
	vkCmdBindIndexBuffer(cmd, quad_mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	int ctr = 0;

	for (auto e : batch->entities)
	{
		auto& particle_system = e.GetComponent<ParticleSystem>();

		if (!particle_system.running)
		{
			continue;
		}

		auto& material = particle_system.material;

		// Bind particle system properties and material index
		ParticleSystemPushConstant pc;
		pc.index = ctr++;

		//pc.material_index = particle_system.material->MaterialIndex();
		vkCmdPushConstants(cmd, _pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ParticleSystemPushConstant), &pc);

		// Bind material textures
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 2, 1, &material->DescriptorSet(), 0, 0);

		// Draw using instancing
		vkCmdDrawIndexed(cmd, quad_mesh->_index_count, particle_system.live_particles, 0, 0, e.Id());
	}
}

void mrs::ParticleRenderPipeline::End(VkCommandBuffer cmd) {}

void mrs::ParticleRenderPipeline::OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch)
{
	uint32_t current_frame = _renderer->CurrentFrame();

	// GRAPHICS ACQUIRE BARRIER: Particle Systems Storage Buffer Memory Barriers
	VkBufferMemoryBarrier particle_parameter_barrier = vkinit::BufferMemoryBarrier(
		_particle_parameter_buffers[current_frame].buffer,
		_particle_parameters_size,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		_renderer->GetQueueIndices().compute,
		_renderer->GetQueueIndices().graphics
	);

	VkBufferMemoryBarrier particle_barrier = vkinit::BufferMemoryBarrier(
		_particle_storage_buffers[current_frame].buffer,
		_particle_buffer_size,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		_renderer->GetQueueIndices().compute,
		_renderer->GetQueueIndices().graphics
	);

	VkBufferMemoryBarrier barriers[] = { particle_parameter_barrier, particle_barrier };
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 2, barriers, 0, nullptr);
}

void mrs::ParticleRenderPipeline::RegisterParticleSystem(ParticleSystem& particle_system)
{
	//  Check next particle offset is within alloted buffer size
	if (_buffer_next_free_offset / _padded_particle_size > _max_particles)
	{
		std::cout << "Max particle count reached!" << std::endl;
		return;
	}

	// See if simillair particle system type exists in gpu cache
	auto cache_it = _particle_system_type_cache.find(particle_system);
	if (cache_it != _particle_system_type_cache.end())
	{
		// Find free buffer offset
		if (!cache_it->second.free_buffer_offsets.empty())
		{
			particle_system.buffer_offset = cache_it->second.free_buffer_offsets.back();
			cache_it->second.free_buffer_offsets.pop_back();
		}
		else
		{
			particle_system.buffer_offset = static_cast<uint32_t>(_buffer_next_free_offset);
			_buffer_next_free_offset += particle_system.max_particles * _padded_particle_size;
		}

		// Copy particle system template to offset
		for (uint32_t i = 0; i < frame_overlaps; i++)
		{
			_renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
				{
					VkBufferCopy region = {};
					region.size = cache_it->second.buffer_size;
					region.srcOffset = cache_it->second.template_instance_buffer_offset;
					region.dstOffset = particle_system.buffer_offset;

					vkCmdCopyBuffer(cmd, _particle_storage_buffers[i].buffer, _particle_storage_buffers[i].buffer, 1, &region);
				});
		}
	}
	else
	{
		// Cache particle system type then register again
		CacheParticleSystemType(particle_system);
		RegisterParticleSystem(particle_system);
		return;
	}

	// Signal registered flag
	particle_system.pipeline = this;
	particle_system.registered = true;
}

void mrs::ParticleRenderPipeline::CacheParticleSystemType(ParticleSystem& particle_system)
{
	// Create new particles data
	std::vector<Particle> particles = {};
	FillParticleArray(particle_system, particles);

	// Create staging buffer for new particles
	AllocatedBuffer staging_buffer = _renderer->CreateBuffer(_padded_particle_size * particle_system.max_particles, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	char* data;
	vmaMapMemory(_renderer->Allocator(), staging_buffer.allocation, (void**)&data);
	for (uint32_t i = 0; i < particle_system.max_particles; i++)
	{
		memcpy(data + (_padded_particle_size * i), &particles[i], sizeof(Particle));
	}
	vmaUnmapMemory(_renderer->Allocator(), staging_buffer.allocation);

	// Cache particle system template in particle buffer
	for (uint32_t i = 0; i < frame_overlaps; i++)
	{
		_renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
			{
				// Copy particle template buffer
				VkBufferCopy region = {};
				region.srcOffset = 0;
				region.size = _padded_particle_size * particle_system.max_particles;
				region.dstOffset = _buffer_next_free_offset;

				vkCmdCopyBuffer(cmd, staging_buffer.buffer, _particle_storage_buffers[i].buffer, 1, &region);
			});
	}

	// Create and cache first instance data 
	ParticleSystemType type;
	type.template_instance_buffer_offset = static_cast<uint32_t>(_buffer_next_free_offset);
	type.buffer_size = particle_system.max_particles * _padded_particle_size;

	_particle_system_type_cache[particle_system] = type;

	// Update buffer offset
	_buffer_next_free_offset += type.buffer_size;

	// Clean up
	vmaDestroyBuffer(_renderer->Allocator(), staging_buffer.buffer, staging_buffer.allocation);
}

void mrs::ParticleRenderPipeline::OnParticleSystemCreated(entt::basic_registry<entt::entity>&, entt::entity entity)
{
	Entity e{ entity, Application::Instance().GetScene() };
}

void mrs::ParticleRenderPipeline::OnParticleSystemDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
{
	Entity e{ entity, Application::Instance().GetScene() };

	// Cache particle system on destruction
	if (e.HasComponent<ParticleSystem>())
	{
		auto& particle_system = e.GetComponent<ParticleSystem>();
		auto cache_it = particle_system.pipeline->_particle_system_type_cache.find(particle_system);

		if (cache_it != particle_system.pipeline->_particle_system_type_cache.end()) {
			cache_it->second.free_buffer_offsets.push_back(particle_system.buffer_offset);
		}
		else
		{
			particle_system.pipeline->CacheParticleSystemType(particle_system);
		}
	}
}

const VkVertexInputBindingDescription& mrs::Particle::GetBinding()
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

