#include "UIRenderPipeline.h"

#include "ECS/Components/Components.h"
#include "Renderer/Vulkan/VulkanInitializers.h"
#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

mrs::UIRenderPipeline::UIRenderPipeline() 
	:IRenderPipeline("UIRenderPipeline")
{
}

mrs::UIRenderPipeline::~UIRenderPipeline() {}

void mrs::UIRenderPipeline::Init() 
{
	InitDescriptors();
	
    Ref<Shader> ui_vertex_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/screen_space_ui_shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    PushShader(ui_vertex_shader);

    Ref<Shader> ui_fragment_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/screen_space_ui_shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    PushShader(ui_fragment_shader);

    _render_pass = _renderer->_offscreen_render_pass;
	BuildPipeline();
    
    // Effect Template
	std::vector<ShaderEffect*> default_ui_effects;
	default_ui_effects.push_back(Effect().get());
	Ref<EffectTemplate> default_ui = VulkanAssetManager::Instance().CreateEffectTemplate(default_ui_effects, "default_ui");

    Material::Create(default_ui, Texture::Get("default"), "default_ui");
    
    // Fonts
    auto ken_pixel = Texture::LoadFromAsset("Assets/Textures/KenPixel.bp");
    Material::Create(default_ui, ken_pixel, "default_ui");
}

void mrs::UIRenderPipeline::InitDescriptors() 
{
    VkDescriptorBufferInfo global_buffer_info = {};
    global_buffer_info.buffer = _renderer->GlobalBuffer().buffer;
    global_buffer_info.offset = 0;
    global_buffer_info.range = VK_WHOLE_SIZE;

    vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
        .BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(&_global_data_set, &_global_data_set_layout);

    _object_sets.resize(frame_overlaps);
    for(uint32_t i = 0; i < frame_overlaps; i++)
    {
        VkDescriptorBufferInfo global_buffer_info = {};
        global_buffer_info.buffer = _renderer->ObjectBuffers()[i].buffer;
        global_buffer_info.offset = 0;
        global_buffer_info.range = VK_WHOLE_SIZE;

        vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
            .BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .Build(&_object_sets[i], &_object_set_layout);
    }
}

void mrs::UIRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
    uint32_t n_frame = _renderer->CurrentFrame();
    VulkanFrameContext frame_context = _renderer->CurrentFrameData();

    // Bind global, object, and light descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

	// Bind ui quad and ui material
	Ref<Mesh> quad_mesh = Mesh::Get("quad");
    VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &quad_mesh->_buffer.buffer, &offset);

	if (quad_mesh->_index_count > 0)
	{
		vkCmdBindIndexBuffer(cmd, quad_mesh->_index_buffer.buffer, offset, VK_INDEX_TYPE_UINT32);
	}

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[n_frame], 0, nullptr);

	for(auto& e : batch->entities)
	{
		auto& renderable = e.GetComponent<Renderable>();
		auto& sprite = e.GetComponent<SpriteRenderer>();

        // Bind material buffer and material texures
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 2, 1, &renderable.material->DescriptorSet(), 0, nullptr);
		vkCmdDrawIndexed(cmd, quad_mesh->_index_count, 1, 0, 0, e.Id());
	}
}

void mrs::UIRenderPipeline::End(VkCommandBuffer cmd) 
{
}
