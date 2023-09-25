#include "VulkanAssetManager.h"
#include "VulkanInitializers.h"

#include "Core/Memory.h"
#include "Renderer/Renderer.h"

#include "Toolbox/FileToolBox.h"

#include "Renderer/RenderPipelineLayers/RenderPipelines/MeshRenderPipeline.h"

namespace mrs
{
	Ref<VulkanAssetManager> VulkanAssetManager::_instance = nullptr;

	void VulkanAssetManager::Init(Renderer* renderer)
	{
		_renderer = renderer;

		// Create texture samplers
		VkSamplerCreateInfo nearest_sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		VK_CHECK(vkCreateSampler(_renderer->Device().device, &nearest_sampler_info, nullptr, &_nearest_image_sampler));

		VkSamplerCreateInfo linear_sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		VK_CHECK(vkCreateSampler(_renderer->Device().device, &linear_sampler_info, nullptr, &_linear_image_sampler));

		// Cache material layout 
		vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
			.BindBuffer(0, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(nullptr, &_material_descriptor_set_layout);

		_renderer->DeletionQueue().Push([&]()
		{
			vkDestroySampler(_renderer->Device().device, _linear_image_sampler, nullptr);
			vkDestroySampler(_renderer->Device().device, _nearest_image_sampler, nullptr);
		});
	}

	void VulkanAssetManager::Shutdown()
	{
	}

	VulkanAssetManager::VulkanAssetManager()
	{
	}

	VulkanAssetManager::~VulkanAssetManager()
	{
	}

	Ref<EffectTemplate> VulkanAssetManager::FindEffectTemplate(const std::string& name)
	{
		if (_effect_templates.find(name) != _effect_templates.end())
		{
			return _effect_templates[name];
		}

		return nullptr;
	}

	Ref<EffectTemplate> VulkanAssetManager::CreateEffectTemplate(const std::vector<ShaderEffect*>& effects, const std::string name)
	{
		if (_effect_templates.find(name) != _effect_templates.end())
		{
			return _effect_templates[name];
		}

		_effect_templates[name] = CreateRef<EffectTemplate>();
		_effect_templates[name]->name = name;
		_effect_templates[name]->shader_effects = effects;

		return _effect_templates[name];
	}

	void VulkanAssetManager::UploadMaterial(Ref<Material> material)
	{
		// Upload material data to material buffer
		size_t padded_material_data_size = _renderer->PadToUniformBufferSize(sizeof(MaterialData));
		AllocatedBuffer staging_buffer = _renderer->CreateBuffer(padded_material_data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		vmaMapMemory(_renderer->Allocator(), staging_buffer.allocation, &data);
		memcpy(data, &material->_data, sizeof(MaterialData));
		vmaUnmapMemory(_renderer->Allocator(), staging_buffer.allocation);

		material->Buffer() = _renderer->CreateBuffer(padded_material_data_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		_renderer->ImmediateSubmit([&](VkCommandBuffer cmd) 
		{
			VkBufferCopy region = {};
			region.srcOffset = 0;
			region.dstOffset = 0;
			region.size = sizeof(MaterialData);
			vkCmdCopyBuffer(cmd, staging_buffer.buffer, material->Buffer().buffer, 1, &region);
		});

		// Build Materials descriptor sets
		VkDescriptorBufferInfo material_buffer_info = {};
		material_buffer_info.buffer = material->Buffer().buffer;
		material_buffer_info.offset = 0;
		material_buffer_info.range = sizeof(MaterialData);

		VkDescriptorImageInfo image_buffer_info = {};
		image_buffer_info.sampler = _linear_image_sampler;
		image_buffer_info.imageView = material->MainTexture()->_image_view;
		image_buffer_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
			.BindBuffer(0, &material_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.BindImage(1, &image_buffer_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(&material->DescriptorSet(), &_material_descriptor_set_layout);
	}

	void VulkanAssetManager::UploadTexture(Ref<Texture> texture)
	{
		void* pixel_ptr = nullptr;

		// Copy to staging buffer
		AllocatedBuffer staging_buffer = _renderer->CreateBuffer(texture->pixel_data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		vmaMapMemory(_renderer->Allocator(), staging_buffer.allocation, &pixel_ptr);
		memcpy(pixel_ptr, texture->pixel_data.data(), texture->pixel_data.size());
		vmaUnmapMemory(_renderer->Allocator(), staging_buffer.allocation);

		// Free pixel data
		texture->pixel_data.clear();
		texture->pixel_data.shrink_to_fit();

		// Create texture
		VkExtent3D extent;
		extent.width = texture->_width;
		extent.height = texture->_height;
		extent.depth = 1;

		VkImageCreateInfo ImageCreateInfo = vkinit::ImageCreateInfo(texture->_format, extent, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

		VmaAllocationCreateInfo vma_alloc_info = {};
		vma_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(_renderer->Allocator(), &ImageCreateInfo, &vma_alloc_info, &texture->_image.image, &texture->_image.allocation, nullptr);

		// Copy data to texture via immediate mode submit
		_renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
			{
				// ~ Transition to transfer optimal
				VkImageSubresourceRange range = {};
				range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				range.layerCount = 1;
				range.baseArrayLayer = 0;
				range.levelCount = 1;
				range.baseMipLevel = 0;

				VkImageMemoryBarrier image_to_transfer_barrier = {};
				image_to_transfer_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				image_to_transfer_barrier.pNext = nullptr;

				image_to_transfer_barrier.image = texture->_image.image;
				image_to_transfer_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				image_to_transfer_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

				image_to_transfer_barrier.srcAccessMask = 0;
				image_to_transfer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				image_to_transfer_barrier.subresourceRange = range;

				vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_to_transfer_barrier);

				// ~ Copy to from staging to texture
				VkBufferImageCopy region = {};
				region.bufferImageHeight = 0;
				region.bufferRowLength = 0;
				region.bufferOffset = 0;

				region.imageExtent = extent;
				region.imageOffset = { 0 };
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;
				region.imageSubresource.mipLevel = 0;

				vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, texture->_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

				// ~ Transition to from shader read only 
				VkImageMemoryBarrier image_to_shader_barrier = image_to_transfer_barrier;
				image_to_shader_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				image_to_shader_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				image_to_shader_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				image_to_shader_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_to_shader_barrier); });

		// Create image view
		VkImageViewCreateInfo image_view_info = vkinit::ImageViewCreateInfo(texture->_image.image, texture->_format, VK_IMAGE_ASPECT_COLOR_BIT);
		VK_CHECK(vkCreateImageView(_renderer->Device().device, &image_view_info, nullptr, &texture->_image_view));

		vmaDestroyBuffer(_renderer->Allocator(), staging_buffer.buffer, staging_buffer.allocation);
		_renderer->DeletionQueue().Push([=]()
		{
			vkDestroyImageView(_renderer->Device().device, texture->_image_view, nullptr);
			vmaDestroyImage(_renderer->Allocator(), texture->_image.image, texture->_image.allocation);
		});
	}

	bool VulkanAssetManager::LoadShaderModule(const char* path, VkShaderModule* module)
	{
		std::vector<char> shader_code;
		tbx::read_file(path, shader_code);

		if (shader_code.data() == nullptr)
		{
			return false;
		}

		VkShaderModuleCreateInfo shader_info = {};
		shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_info.pNext = nullptr;

		shader_info.codeSize = shader_code.size();
		shader_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());
		shader_info.flags = 0;

		if (vkCreateShaderModule(_renderer->Device().device, &shader_info, nullptr, module) != VK_SUCCESS)
		{
			return false;
		}

		_renderer->DeletionQueue().Push([&]()
		{
			vkDestroyShaderModule(_renderer->Device().device, *module, nullptr);
		});

		return true;
	}
	const Ref<Shader> VulkanAssetManager::LoadShader(const char* path, VkShaderStageFlagBits stage)
	{
		// TODO: Store shaders in cache
		Ref<Shader> shader = CreateRef<Shader>();

		tbx::read_file(path, shader->byte_code);

		if (shader->byte_code.data() == nullptr)
		{
			return nullptr;
		}

		VkShaderModuleCreateInfo shader_info = {};
		shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_info.pNext = nullptr;

		shader_info.codeSize = shader->byte_code.size();
		shader_info.pCode = reinterpret_cast<const uint32_t*>(shader->byte_code.data());
		shader_info.flags = 0;

		if (vkCreateShaderModule(_renderer->Device().device, &shader_info, nullptr, &shader->shader_module) != VK_SUCCESS)
		{
			return nullptr;
		}

		std::string path_str = std::string(path);
		shader->name = path_str.substr(path_str.find_last_of('/') + 1);
		shader->stage = stage;
		return shader;
	}
}