#include "TerrainRenderPipeline.h"
#include <ECS/Components/Components.h>
#include <Renderer/Vulkan/VulkanInitializers.h>

void mrs::TerrainRenderPipeline::Init()
{
    _quad_mesh = Mesh::Get("quad");

    // Load height maps
    auto view = _scene->Registry()->view<Transform, TerrainRenderer>();
    for (entt::entity entity : view)
    {
        Entity e = { entity , _scene };
        TerrainRenderer &terrain = e.GetComponent<TerrainRenderer>();
        std::shared_ptr<Texture> height_map = terrain.height_map;

        // Create and upload mesh 
        terrain._terrain_mesh = Mesh::Create("terrain_mesh");

        for (uint32_t i = 0; i < terrain.height_map->_height; i++)
        {
            for (uint32_t j = 0; j < terrain.height_map->_width; j++)
            {
                Vertex vert = {};
                vert.position.x = -(static_cast<float>(terrain.height_map->_height)) / 2.0f + i;

                // to be termined using height map
                vert.position.y = { 0 };

                vert.position.z = -(static_cast<float>(terrain.height_map->_width)) / 2.0f + j;

                terrain._terrain_mesh->_vertices.push_back(vert);
            }
        }
        for (uint32_t i = 0; i < terrain.height_map->_height - 1; i++)
        {
            for (uint32_t j = 0; j < terrain.height_map->_width; j++)
            {
                for(uint32_t k = 0; k < 2; k++)
                {
                    terrain._terrain_mesh->_indices.push_back(j + (terrain.height_map->_width * (i + k)));
                }
            }
        }

        terrain._terrain_mesh->_vertex_count = terrain._terrain_mesh->_vertices.size();
        terrain._terrain_mesh->_index_count = terrain._terrain_mesh->_indices.size();
        _renderer->UploadMesh(terrain._terrain_mesh);

        // Create terrain descriptors
        VkDescriptorImageInfo height_map_descriptor_info = {};
        height_map_descriptor_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        height_map_descriptor_info.imageView = height_map->_image_view;
        height_map_descriptor_info.sampler = _asset_manager->GetLinearImageSampler();

        vkutil::DescriptorBuilder _descriptor_builder;
        _descriptor_builder.Begin(_renderer->_descriptor_layout_cache.get(), _renderer->_descriptor_allocator.get())
            .BindImage(0, &height_map_descriptor_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT)
            .Build(&_terrain_descriptor_set, &_terrain_descriptor_set_layout);
    }

    CreateTerrainPipelineLayout();
    CreateTerrainPipeline();
}

void mrs::TerrainRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame)
{
    VkDescriptorSet _frame_object_set = _renderer->GetCurrentGlobalObjectDescriptorSet();
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _terrain_render_pipeline);

    auto view = _scene->Registry()->view<Transform, TerrainRenderer>();

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _terrain_render_pipeline_layout, 0, 1, &_global_descriptor_set, 0, 0);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _terrain_render_pipeline_layout, 1, 1, &_frame_object_set, 0, 0);

    std::shared_ptr<Material> default_material = Material::Get("default_material");
    for (auto entity : view)
    {
        Entity e = { entity, _scene };
        auto &terrain_renderer = e.GetComponent<TerrainRenderer>();

        // Bind material buffer and material textures
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _terrain_render_pipeline_layout, 2, 1, &default_material->material_descriptor_set, 0, 0);
        TerrainGraphicsPushConstant terrain_push_constant = {};
        terrain_push_constant.material_index = default_material->GetMaterialIndex();
        vkCmdPushConstants(cmd, _terrain_render_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(TerrainGraphicsPushConstant), &terrain_push_constant);

        // Bind terrain properties
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _terrain_render_pipeline_layout, 3, 1, &_terrain_descriptor_set, 0, 0);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &terrain_renderer._terrain_mesh->_buffer.buffer, &offset);
        vkCmdBindIndexBuffer(cmd, terrain_renderer._terrain_mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, terrain_renderer._terrain_mesh->_index_count, 1, 0, 0, e.Id());
    }
}

void mrs::TerrainRenderPipeline::End(VkCommandBuffer cmd)
{

}

void mrs::TerrainRenderPipeline::CreateTerrainPipelineLayout()
{
    std::vector<VkDescriptorSetLayout> set_layouts = { _global_descriptor_set_layout, _object_descriptor_set_layout, _asset_manager->GetMaterialDescriptorSetLayout(), _terrain_descriptor_set_layout };

    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(TerrainGraphicsPushConstant);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();
    pipeline_layout_info.setLayoutCount = set_layouts.size();
    pipeline_layout_info.pSetLayouts = set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant;

    VK_CHECK(vkCreatePipelineLayout(_device->device, &pipeline_layout_info, nullptr, &_terrain_render_pipeline_layout));
}

void mrs::TerrainRenderPipeline::CreateTerrainPipeline()
{
    vkutil::PipelineBuilder pipeline_builder = {};

    // Pipeline view port
    pipeline_builder._scissor.extent = { _window->GetWidth(),
                                        _window->GetHeight()};
    pipeline_builder._scissor.offset = { 0, 0 };

    pipeline_builder._viewport.x = 0.0f;
    pipeline_builder._viewport.y = 0.0f;
    pipeline_builder._viewport.width = (float)(_window->GetWidth());
    pipeline_builder._viewport.height = (float)(_window->GetHeight());
    pipeline_builder._viewport.minDepth = 0.0f;
    pipeline_builder._viewport.maxDepth = 1.0f;

    // Shaders modules
    bool loaded = false;
    VkShaderModule vertex_shader_module, fragment_shader_module = {};
    loaded = _renderer->LoadShaderModule("Assets/Shaders/terrain_shader.vert.spv", &vertex_shader_module);
    loaded = _renderer->LoadShaderModule("Assets/Shaders/terrain_shader.frag.spv", &fragment_shader_module);

    pipeline_builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader_module));
    pipeline_builder._shader_stages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader_module));

    // Vertex input (Primitives and Vertex Input Descriptions
    pipeline_builder._input_assembly = vkinit::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    VertexInputDescription &vb_desc = Vertex::GetDescription();

    pipeline_builder._vertex_input_info = vkinit::pipeline_vertex_input_state_create_info();

    pipeline_builder._vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vb_desc.bindings.size());
    pipeline_builder._vertex_input_info.pVertexBindingDescriptions = vb_desc.bindings.data();

    pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vb_desc.attributes.size());
    pipeline_builder._vertex_input_info.pVertexAttributeDescriptions = vb_desc.attributes.data();

    // Graphics Settings
    pipeline_builder._rasterizer = vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL);
    pipeline_builder._multisampling = vkinit::pipeline_mulitisample_state_create_info();
    pipeline_builder._color_blend_attachment = vkinit::pipeline_color_blend_attachment_state();
    pipeline_builder._depth_stencil = vkinit::pipeline_depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    pipeline_builder._pipeline_layout = _terrain_render_pipeline_layout;

    _terrain_render_pipeline = pipeline_builder.Build(_device->device, _renderer->_offscreen_render_pass);

    if (_terrain_render_pipeline_layout == VK_NULL_HANDLE)
    {
        printf("Failed to create pipeline");
    }

    // Clean Up
    vkDestroyShaderModule(_device->device, fragment_shader_module, nullptr);
    vkDestroyShaderModule(_device->device, vertex_shader_module, nullptr);
    _renderer->GetDeletionQueue().Push([=]()
        {
            vkDestroyPipelineLayout(_device->device, _terrain_render_pipeline_layout, nullptr);
            vkDestroyPipeline(_device->device, _terrain_render_pipeline, nullptr);
        });
}
