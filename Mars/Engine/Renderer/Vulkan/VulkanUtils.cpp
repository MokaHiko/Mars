#include "VulkanUtils.h"

#include <algorithm>

VkPipeline vkutil::PipelineBuilder::Build(VkDevice device, VkRenderPass renderPass, bool offscreen)
{
	VkPipelineViewportStateCreateInfo view_port_state = {};
	view_port_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	view_port_state.viewportCount = 1;
	view_port_state.pViewports = &_viewport;
	view_port_state.scissorCount = 1;
	view_port_state.pScissors = &_scissor;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = offscreen ? 0 : 1;
	color_blending.pAttachments = &_color_blend_attachment;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipeline_info.stageCount = static_cast<uint32_t>(_shader_stages.size());
	pipeline_info.pStages = _shader_stages.data();
	pipeline_info.pVertexInputState = &_vertex_input_info;
	pipeline_info.pInputAssemblyState = &_input_assembly;
	pipeline_info.pRasterizationState = &_rasterizer;
	pipeline_info.pMultisampleState = &_multisampling;
	pipeline_info.pDepthStencilState = &_depth_stencil;
	pipeline_info.layout = _pipeline_layout;
	pipeline_info.pTessellationState = &_tesselation_state;
	pipeline_info.pViewportState = &view_port_state;
	pipeline_info.pColorBlendState = &color_blending;

	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.renderPass = renderPass;
	pipeline_info.subpass = 0;

	VkPipeline new_pipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) != VK_SUCCESS)
	{
		printf("Failed to create graphics pipeline!\n");
		return VK_NULL_HANDLE;
	}

	return new_pipeline;
}

void vkutil::DescriptorAllocator::Init(VkDevice device)
{
	_device_h = device;
}

void vkutil::DescriptorAllocator::CleanUp()
{
	for (auto p : _free_pools) {
		vkDestroyDescriptorPool(_device_h, p, nullptr);
	}

	for (auto p : _used_pools) {

		vkDestroyDescriptorPool(_device_h, p, nullptr);
	}
}

bool vkutil::DescriptorAllocator::Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
{
	if (_current_pool == VK_NULL_HANDLE) {
		_current_pool = GetPool();
		_used_pools.push_back(_current_pool);
	}

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;

	alloc_info.descriptorPool = _current_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &layout;

	VkResult result = vkAllocateDescriptorSets(_device_h, &alloc_info, set);
	bool re_alloc = false;

	switch (result) {
	case VK_SUCCESS:
	{
		return true;
		break;
	}
	case VK_ERROR_FRAGMENTED_POOL:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
	{
		re_alloc = true;
		break;
	}
	}

	if (re_alloc) {
		// Store in used pools
		_used_pools.push_back(_current_pool);

		// Create new pool and try to allocate again
		_current_pool = GetPool();
		alloc_info.descriptorPool = _current_pool;
		if (vkAllocateDescriptorSets(_device_h, &alloc_info, set) == VK_SUCCESS) {
			return true;
		};
	}

	return false;
}

VkDescriptorPool vkutil::DescriptorAllocator::GetPool()
{
	if (_free_pools.size() > 0) {
		VkDescriptorPool pool = _free_pools.back();
		_free_pools.pop_back();

		return pool;
	}

	return create_pool(_device_h, descriptor_pool_sizes, 1000, 0);
}

void vkutil::DescriptorAllocator::Reset()
{
	for (auto p : _used_pools) {
		vkResetDescriptorPool(_device_h, p, 0);
		_free_pools.push_back(p);
	}

	_used_pools.clear();
	_current_pool = VK_NULL_HANDLE;
}

VkDescriptorPool vkutil::create_pool(VkDevice device, const vkutil::DescriptorAllocator::PoolSizes& pool_sizes, uint32_t count, VkDescriptorPoolCreateFlags create_flags)
{
	VkDescriptorPoolCreateInfo descriptor_pool_info = {};
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.pNext = nullptr;

	descriptor_pool_info.flags = create_flags;

	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(pool_sizes.sizes.size());

	for (auto& size : pool_sizes.sizes) {
		sizes.push_back({ size.first, (uint32_t)(size.second * count) });
	}

	descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(sizes.size());
	descriptor_pool_info.pPoolSizes = sizes.data();
	descriptor_pool_info.maxSets = count;

	VkDescriptorPool new_pool;
	vkCreateDescriptorPool(device, &descriptor_pool_info, nullptr, &new_pool);

	return new_pool;
}

void vkutil::DescriptorLayoutCache::Init(VkDevice device)
{
	_device_h = device;
}

VkDescriptorSetLayout vkutil::DescriptorLayoutCache::CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo* info)
{
	DescriptorLayoutInfo layout_info = {};
	layout_info.bindings.reserve(info->bindingCount);

	bool is_sorted = true;

	uint32_t last_binding = -1;

	// Copy to layout_info struct to check/store in cache
	for (uint32_t i = 0; i < info->bindingCount; i++) {
		layout_info.bindings.push_back(info->pBindings[i]);

		if (last_binding > layout_info.bindings[i].binding) {
			is_sorted = false;
		}
	}

	// Sort by binding
	if (!is_sorted) {
		std::sort(layout_info.bindings.begin(), layout_info.bindings.end(), [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) {
			return a.binding < b.binding;
			});
	}

	// Find in cache
	auto it = _cache.find(layout_info);
	if (it != _cache.end()) {
		return _cache[layout_info];
	}

	// Create and store otherwise
	VkDescriptorSetLayout layout;
	vkCreateDescriptorSetLayout(_device_h, info, nullptr, &layout);

	_cache[layout_info] = layout;
	return _cache[layout_info];
}

void vkutil::DescriptorLayoutCache::Clear()
{
	for (auto& pair : _cache) {
		vkDestroyDescriptorSetLayout(_device_h, pair.second, nullptr);
	}
}

size_t vkutil::DescriptorLayoutCache::DescriptorLayoutInfo::Hash() const
{
	// Hash the size
	size_t result = std::hash<size_t>()(bindings.size());

	// Has each binding
	for (const VkDescriptorSetLayoutBinding& b : bindings) {

		size_t binding_hash_bit = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;


		// Shufle the binding data and xor with main hash
		result ^= std::hash<size_t>()(binding_hash_bit);
	}

	return result;
}

bool vkutil::DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
{
	if (bindings.size() != other.bindings.size()) {
		return false;
	}

	for (uint32_t i = 0; i < bindings.size(); i++) {
		if (bindings[i].binding != other.bindings[i].binding) {
			return false;
		}

		if (bindings[i].descriptorCount != other.bindings[i].descriptorCount) {
			return false;
		}

		if (bindings[i].descriptorType != other.bindings[i].descriptorType) {
			return false;
		}

		if (bindings[i].stageFlags != other.bindings[i].stageFlags) {
			return false;
		}
	}

	return true;
}

vkutil::DescriptorBuilder& vkutil::DescriptorBuilder::BindBuffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info, VkDescriptorType type, VkShaderStageFlags shader_stage)
{
	// Push binding
	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = binding;
	layout_binding.descriptorType = type;
	layout_binding.stageFlags = shader_stage;

	layout_binding.descriptorCount = 1;

	_bindings.push_back(layout_binding);

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.descriptorType = type;
	write.descriptorCount = 1;
	write.pBufferInfo = buffer_info;
	write.dstBinding = binding;

	_writes.push_back(write);

	return *this;
}

vkutil::DescriptorBuilder& vkutil::DescriptorBuilder::BindImage(uint32_t binding, VkDescriptorImageInfo* image_info, VkDescriptorType type, VkShaderStageFlags shader_stage)
{
	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = binding;
	layout_binding.descriptorType= type;
	layout_binding.stageFlags = shader_stage;

	layout_binding.pImmutableSamplers = nullptr;

	layout_binding.descriptorCount = 1;

	_bindings.push_back(layout_binding);

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.descriptorType = type;
	write.descriptorCount = 1;
	write.pImageInfo = image_info;
	write.dstBinding = binding;

	_writes.push_back(write);

	return *this;
}

bool vkutil::DescriptorBuilder::Build(VkDescriptorSet* set, VkDescriptorSetLayout* layout) {

	// Create layout
	VkDescriptorSetLayoutCreateInfo layout_create_info = {};
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = nullptr;

	layout_create_info.bindingCount = static_cast<uint32_t>(_bindings.size());
	layout_create_info.pBindings = _bindings.data();

	*layout = _cache->CreateDescriptorSetLayout(&layout_create_info);

	// Check if only caching layout
	if(!set) 
	{
		return false;
	}

	// Allocate descriptor 
	if (!(_allocator->Allocate(set, *layout))) {
		return false;
	}

	// Assign descriptor set to each write
	for (auto& write : _writes) {
		write.dstSet = *set;
	}

	// Update descriptors
	vkUpdateDescriptorSets(_allocator->_device_h, static_cast<uint32_t>(_writes.size()), _writes.data(), 0, nullptr);

	return true;
}

