#include "MeshRenderPipeline.h"

#include "ECS/Components/Components.h"
#include "Renderer/Vulkan/VulkanInitializers.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

void mrs::MeshRenderPipeline::InitDescriptors() {
    VkDescriptorBufferInfo global_buffer_info = {};
    global_buffer_info.buffer = _renderer->GlobalBuffer().buffer;
    global_buffer_info.offset = 0;
    global_buffer_info.range = VK_WHOLE_SIZE;

    vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
        .BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(&_global_data_set, &_global_data_set_layout);

    _object_sets.resize(frame_overlaps);
    _dir_light_sets.resize(frame_overlaps);
    for (uint32_t i = 0; i < frame_overlaps; i++)
    {
        VkDescriptorBufferInfo object_buffer_info = {};
        object_buffer_info.buffer = _renderer->ObjectBuffers()[i].buffer;
        object_buffer_info.offset = 0;
        object_buffer_info.range = VK_WHOLE_SIZE;

        vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
            .BindBuffer(0, &object_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .Build(&_object_sets[i], &_object_set_layout);

        VkDescriptorBufferInfo dir_light_buffer_info = {};
        dir_light_buffer_info.buffer = _renderer->DirLightBuffers()[i].buffer;
        dir_light_buffer_info.offset = 0;
        dir_light_buffer_info.range = VK_WHOLE_SIZE;

        vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
            .BindBuffer(0, &dir_light_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build(&_dir_light_sets[i], &_dir_light_layout);
    }

    VkDescriptorImageInfo shadow_map_image_info = {};
    shadow_map_image_info.sampler = _shadow_map_sampler;
    shadow_map_image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    shadow_map_image_info.imageView = _offscreen_depth_image_view;

    vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
        .BindImage(0, &shadow_map_image_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(&_shadow_map_descriptor, &_shadow_map_descriptor_layout);
}

mrs::MeshRenderPipeline::MeshRenderPipeline()
    :IRenderPipeline("MeshRenderPipeline") {}

mrs::MeshRenderPipeline::~MeshRenderPipeline() {}

void mrs::MeshRenderPipeline::Init() {
    // Shadow Pipeline
    CreateOffScreenFramebuffer();
    InitDescriptors();
    InitOffScreenPipeline();

    // Reflection based descriptor set/layout creation
    Ref<Shader> mesh_vertex_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/default_shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    PushShader(mesh_vertex_shader);

    Ref<Shader> mesh_fragment_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/default_shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    PushShader(mesh_fragment_shader);

    // TODO: Add as defaut in "Mesh pipeline type render pipeline"
    _render_pass = _renderer->_offscreen_render_pass;
    BuildPipeline();

    // Indirect drawing
    InitIndirectCommands();

    // Define Effect Template
    std::vector<ShaderEffect*> default_lit_effects;
    default_lit_effects.push_back(Effect().get());
    Ref<EffectTemplate> default_lit = VulkanAssetManager::Instance().CreateEffectTemplate(default_lit_effects, "default_lit");

    // Default Mesh Materials
    Ref<Texture> default_texture = Texture::Get("default");
    Material::Create(default_lit, default_texture, "default");
    Material::Create(default_lit, default_texture, "red")->DiffuseColor() = { 1.0f, 0.0f, 0.0f, 1.0f };
    Material::Create(default_lit, default_texture, "green")->DiffuseColor() = { 0.0f, 1.0f, 0.0f, 1.0f };
    Material::Create(default_lit, default_texture, "blue")->DiffuseColor() = { 0.0f, 0.0f, 1.0f, 1.0f };

    Mesh::LoadFromAsset("Assets/Models/cube.boop_obj", "cube");
    Mesh::LoadFromAsset("Assets/Models/cone.boop_obj", "cone");
    Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");
    Mesh::LoadFromAsset("Assets/Models/quad.boop_obj", "quad");
    Mesh::LoadFromAsset("Assets/Models/plane.boop_obj", "plane");
    Mesh::LoadFromAsset("Assets/Models/sphere.boop_obj", "sphere");

    Mesh::LoadFromAsset("Assets/Models/container.boop_obj", "container");
    Mesh::LoadFromAsset("Assets/Models/soldier.boop_obj", "soldier");

    // Manually built meshes
    auto screen_quad = Mesh::Create("screen_quad");
    screen_quad->_vertices.push_back({ { -1.0f, -1.0f, 0.0f }, {}, {}, {} });
    screen_quad->_vertices.push_back({ { -1.0f, 1.0f, 0.0f }, {}, {}, {} });
    screen_quad->_vertices.push_back({ { 1.0f, -1.0f, 0.0f }, {}, {}, {} });
    screen_quad->_vertices.push_back({ { 1.0f,  1.0f, 0.0f }, {}, {}, {} });
    screen_quad->_vertex_count = 4;

    screen_quad->_indices = { 0,2,1,1,2,3 };
    screen_quad->_index_count = 6;
}

void mrs::MeshRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
    if (_rerecord)
    {
        BuildBatches(cmd, batch);
        RecordIndirectcommands(cmd, batch);

        _rerecord = false;
    }

    DrawObjects(cmd, current_frame, batch);
}

void mrs::MeshRenderPipeline::End(VkCommandBuffer cmd)
{
}

void mrs::MeshRenderPipeline::OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch)
{
    // Draw to offscreen frame buffer for shadow map
    VkRect2D area = {};
    area.extent = { _window->GetWidth(), _window->GetHeight() };
    area.offset = { 0, 0 };

    VkClearValue depth_value = {};
    depth_value.depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo offscreen_render_pass_begin_info = vkinit::RenderPassBeginInfo(_offscreen_framebuffer, _offscreen_render_pass, area, &depth_value, 1);
    vkCmdBeginRenderPass(cmd, &offscreen_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    DrawShadowMap(cmd, batch);
    vkCmdEndRenderPass(cmd);
}


void mrs::MeshRenderPipeline::OnMaterialsUpdate()
{
    _rerecord = true;
}

void mrs::MeshRenderPipeline::OnRenderableCreated(Entity e)
{
    _rerecord = true;
}

void mrs::MeshRenderPipeline::OnRenderableDestroyed(Entity e)
{
    _rerecord = true;
}

void mrs::MeshRenderPipeline::DrawObjects(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
    VulkanFrameContext frame_context = _renderer->CurrentFrameData();

    // Bind global, object, and light descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[current_frame], 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 4, 1, &_dir_light_sets[current_frame], 0, nullptr);

    for (auto& batch : _batches)
    {
        // Bind material buffer and material texures
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 2, 1, &batch.material->DescriptorSet(), 0, nullptr);

        // Bind shadow map
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 3, 1, &_shadow_map_descriptor, 0, nullptr);

        // Bind batch mesh vertex and index buffers
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &batch.mesh->_buffer.buffer, &offset);

        if (batch.mesh->_index_count > 0)
        {
            vkCmdBindIndexBuffer(cmd, batch.mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        }

        // Draw batch
        static uint32_t batch_stride = static_cast<uint32_t>(_renderer->PadToStorageBufferSize(sizeof(VkDrawIndexedIndirectCommand)));
        uint32_t indirect_offset = batch.first * batch_stride;
        vkCmdDrawIndexedIndirect(cmd, _indirect_buffers[current_frame].buffer, indirect_offset, batch.count, batch_stride);
    }
}

void mrs::MeshRenderPipeline::InitIndirectCommands()
{
    size_t max_indirect_commands = 1000;

    // Create indirect buffer per frame
    _indirect_buffers.resize(frame_overlaps);
    for (int i = 0; i < frame_overlaps; i++)
    {
        _indirect_buffers[i] =
            _renderer->CreateBuffer(_renderer->PadToStorageBufferSize(
                _renderer->PadToStorageBufferSize(sizeof(VkDrawIndexedIndirectCommand))) *
                max_indirect_commands,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU, 0);

        _renderer->DeletionQueue().Push([=]()
            {
                vmaDestroyBuffer(_renderer->Allocator(), _indirect_buffers[i].buffer, _indirect_buffers[i].allocation);
            });
    }
}

std::vector<mrs::IndirectBatch> mrs::MeshRenderPipeline::GetRenderablesAsBatches(RenderableBatch* batch)
{
    std::vector<IndirectBatch> batches;

    Material* last_material = nullptr;
    Mesh* last_mesh = nullptr;

    int batch_first = 0;

    for (auto e : batch->entities)
    {
        auto& renderable = e.GetComponent<MeshRenderer>();

        // Check if new batch
        bool new_batch = renderable.GetMaterial().get() != last_material || renderable.GetMesh().get() != last_mesh;
        if (new_batch)
        {
            IndirectBatch batch = {};

            batch.count = 1;
            batch.material = renderable.GetMaterial().get();
            batch.mesh = renderable.GetMesh().get();

            // Increment batch first by entiteis in last batch
            if (!batches.empty())
            {
                batch_first += batches.back().count;
            }

            batch.first = batch_first;
            batches.push_back(batch);

            last_mesh = renderable.GetMesh().get();
            last_material = renderable.GetMaterial().get();
        }
        else
        {
            batches.back().count++;
        }
    }

    return batches;
}

void mrs::MeshRenderPipeline::InitOffScreenPipeline()
{
    vkutil::PipelineBuilder pipeline_builder = {};

    // Pipeline view port
    pipeline_builder._scissor.extent = { _window->GetWidth(),
                                        _window->GetHeight() };
    pipeline_builder._scissor.offset = { 0, 0 };

    pipeline_builder._viewport.x = 0.0f;
    pipeline_builder._viewport.y = 0.0f;
    pipeline_builder._viewport.width = (float)(_window->GetWidth());
    pipeline_builder._viewport.height = (float)(_window->GetHeight());
    pipeline_builder._viewport.minDepth = 0.0f;
    pipeline_builder._viewport.maxDepth = 1.0f;

    // Shaders modules
    bool loaded = false;
    VkShaderModule vertex_shader_module;
    loaded = VulkanAssetManager::Instance().LoadShaderModule("Assets/Shaders/offscreen_shader.vert.spv", &vertex_shader_module);
    pipeline_builder._shader_stages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader_module));

    // Vertex input (Primitives and Vertex Input Descriptions)
    pipeline_builder._input_assembly = vkinit::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    VertexInputDescription& vb_desc = Vertex::GetDescription();

    pipeline_builder._vertex_input_info = vkinit::PipelineVertexInputStateCreateInfo();

    pipeline_builder._vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vb_desc.bindings.size());
    pipeline_builder._vertex_input_info.pVertexBindingDescriptions = vb_desc.bindings.data();

    pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vb_desc.attributes.size());
    pipeline_builder._vertex_input_info.pVertexAttributeDescriptions = vb_desc.attributes.data();

    // Disable cull so all faces contribute to shadows
    pipeline_builder._rasterizer = vkinit::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    pipeline_builder._rasterizer.cullMode = VK_CULL_MODE_NONE;

    pipeline_builder._multisampling = vkinit::PipelineMultisampleStateCreateInfo();
    pipeline_builder._color_blend_attachment = {};
    pipeline_builder._depth_stencil = vkinit::PipelineDepthStencilStateCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();
    std::vector<VkDescriptorSetLayout> descriptor_layouts = { _global_data_set_layout, _object_set_layout };
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size());
    pipeline_layout_info.pSetLayouts = descriptor_layouts.data();

    VK_CHECK(vkCreatePipelineLayout(_device->device, &pipeline_layout_info, nullptr, &_offscreen_render_pipeline_layout));
    pipeline_builder._pipeline_layout = _offscreen_render_pipeline_layout;

    _offscreen_render_pipeline = pipeline_builder.Build(_device->device, _offscreen_render_pass, true);

    if (_offscreen_render_pipeline == VK_NULL_HANDLE)
    {
        printf("Failed to create pipeline!");
    }

    // Clean Up
    vkDestroyShaderModule(_device->device, vertex_shader_module, nullptr);
    _renderer->DeletionQueue().Push([=]()
        {
            vkDestroyPipeline(_device->device, _offscreen_render_pipeline, nullptr);
            vkDestroyPipelineLayout(_device->device, _offscreen_render_pipeline_layout, nullptr);
        });
}

void mrs::MeshRenderPipeline::BuildBatches(VkCommandBuffer cmd, RenderableBatch* batch)
{
    _batches = GetRenderablesAsBatches(batch);
}

void mrs::MeshRenderPipeline::RecordIndirectcommands(VkCommandBuffer cmd, RenderableBatch* batch)
{
    uint32_t frame_index = _renderer->CurrentFrame();
    char* draw_commands;
    vmaMapMemory(_renderer->Allocator(), _indirect_buffers[frame_index].allocation, (void**)&draw_commands);

    // Encode draw commands for each renderable ahead of time
    int ctr = 0;
    for (auto e : batch->entities)
    {
        auto renderable = e.GetComponent<MeshRenderer>();

        VkDrawIndexedIndirectCommand draw_command = {};
        draw_command.vertexOffset = 0;
        draw_command.indexCount = renderable.GetMesh()->_index_count;
        draw_command.instanceCount = 1;
        draw_command.firstInstance = e.Id();

        static size_t padded_draw_indirect_size = _renderer->PadToStorageBufferSize(sizeof(VkDrawIndexedIndirectCommand));
        size_t offset = ctr * padded_draw_indirect_size;
        memcpy(draw_commands + offset, &draw_command, sizeof(VkDrawIndexedIndirectCommand));
        ctr++;
    }
    vmaUnmapMemory(_renderer->Allocator(), _indirect_buffers[frame_index].allocation);
}

void mrs::MeshRenderPipeline::DrawShadowMap(VkCommandBuffer cmd, RenderableBatch* batch)
{
    uint32_t n_frame = _renderer->CurrentFrame();

    // Bind global and object descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _offscreen_render_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[n_frame], 0, nullptr);

    for (auto& batch : _batches)
    {
        // Bind batch vertex and index buffer
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &batch.mesh->_buffer.buffer, &offset);

        if (batch.mesh->_index_count > 0)
        {
            vkCmdBindIndexBuffer(cmd, batch.mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        }

        // Draw batch
        static uint32_t batch_stride = static_cast<uint32_t>(_renderer->PadToStorageBufferSize(sizeof(VkDrawIndexedIndirectCommand)));
        uint32_t indirect_offset = batch.first * batch_stride;

        vkCmdDrawIndexedIndirect(cmd, _indirect_buffers[n_frame].buffer, indirect_offset, batch.count, batch_stride);
    }
}

void mrs::MeshRenderPipeline::CreateOffScreenFramebuffer()
{
    // Create frame buffer attachment
    VkExtent3D extent = {};
    extent.depth = 1;
    extent.width = _window->GetWidth();
    extent.height = _window->GetHeight();

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
    VkImageCreateInfo depth_image_info = vkinit::ImageCreateInfo(depth_format, extent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VmaAllocationCreateInfo vmaaloc_info = {};
    vmaaloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaaloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vmaCreateImage(_renderer->Allocator(), &depth_image_info,
        &vmaaloc_info, &_offscreen_depth_image.image,
        &_offscreen_depth_image.allocation, nullptr));

    // Create offscreen attachment view
    VkImageViewCreateInfo depth_image_view_info = vkinit::ImageViewCreateInfo(
        _offscreen_depth_image.image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(_device->device, &depth_image_view_info, nullptr,
        &_offscreen_depth_image_view));

    // Create render pass
    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depth_attachment_reference = {};
    depth_attachment_reference.attachment = 0;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pDepthStencilAttachment = &depth_attachment_reference;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;

    std::array<VkSubpassDependency, 2> dependencies = {};

    // Between external subpass and current subpass
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // reading of last frame
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL; // our color render pass
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // after depth testing has

    // finished
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // before shader reads data
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo _offscreen_render_pass_info = {};
    _offscreen_render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    _offscreen_render_pass_info.attachmentCount = 1;
    _offscreen_render_pass_info.pAttachments = &depth_attachment;

    // subpasses
    _offscreen_render_pass_info.subpassCount = 1;
    _offscreen_render_pass_info.pSubpasses = &subpass;

    // dependencies between subpasses
    _offscreen_render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    _offscreen_render_pass_info.pDependencies = dependencies.data();

    VK_CHECK(vkCreateRenderPass(_device->device, &_offscreen_render_pass_info, nullptr, &_offscreen_render_pass));

    // Create Frame buffer
    VkFramebufferCreateInfo offscreen_framebuffer_info = {};
    offscreen_framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    offscreen_framebuffer_info.renderPass = _offscreen_render_pass;

    offscreen_framebuffer_info.attachmentCount = 1;
    offscreen_framebuffer_info.width = _window->GetWidth();
    offscreen_framebuffer_info.height = _window->GetHeight();
    offscreen_framebuffer_info.pAttachments = &_offscreen_depth_image_view;
    offscreen_framebuffer_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(_device->device, &offscreen_framebuffer_info,
        nullptr, &_offscreen_framebuffer));

    // Shadow map image sampeler
    VkSamplerCreateInfo shadow_map_sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    shadow_map_sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    shadow_map_sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    shadow_map_sampler_info.mipLodBias = 0.0f;
    shadow_map_sampler_info.maxAnisotropy = 1.0f;
    shadow_map_sampler_info.minLod = 0.0f;
    shadow_map_sampler_info.maxLod = 1.0f;

    VK_CHECK(vkCreateSampler(_device->device, &shadow_map_sampler_info, nullptr,
        &_shadow_map_sampler));

    _renderer->DeletionQueue().Push([=]()
        {
            vkDestroySampler(_device->device, _shadow_map_sampler, nullptr);
            vkDestroyFramebuffer(_device->device, _offscreen_framebuffer, nullptr);
            vkDestroyRenderPass(_device->device, _offscreen_render_pass, nullptr);
            vkDestroyImageView(_device->device, _offscreen_depth_image_view, nullptr);
            vmaDestroyImage(_renderer->Allocator(), _offscreen_depth_image.image, _offscreen_depth_image.allocation);
        });
}

