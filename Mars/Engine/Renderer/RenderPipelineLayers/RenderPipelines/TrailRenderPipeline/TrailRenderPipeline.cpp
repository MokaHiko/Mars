#include "TrailRenderPipeline.h"

#include "Core/Application.h"
#include "Trails.h"

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

mrs::TrailRenderPipeline::TrailRenderPipeline()
    :IRenderPipeline("TrailRenderPipeline") {}

mrs::TrailRenderPipeline::~TrailRenderPipeline() {}

void mrs::TrailRenderPipeline::OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch)
{
    int current_index = _renderer->CurrentFrame();
    for (auto entity : batch->entities)
    {
        Entity e(entity, _scene);
        auto& transform = e.GetComponent<mrs::Transform>();
        auto& trail_renderer = e.GetComponent<TrailRenderer>();

        if(glm::length(trail_renderer.last_position - transform.position) >= trail_renderer.min_vertex_distance)
        {
            Vertex v = {};
            v.position = transform.position;
            trail_renderer.points.push_back(v);
            trail_renderer.last_position = transform.position;

            for(int i = 0; i < frame_overlaps; i++)
            {
                // Switch to single staging buffer copy of the entire thing
                vkCmdUpdateBuffer(cmd, _trails_vertex_buffer[i].buffer, trail_renderer.vertex_offset + (sizeof(Vertex)* (trail_renderer.points.size() - 1)), sizeof(Vertex), &trail_renderer.points.back());
            }
        }
    }

    VkBufferMemoryBarrier vertex_buffer_barriers[frame_overlaps];
    for(int i = 0; i <frame_overlaps; i++)
    {
        vertex_buffer_barriers[i] = vkinit::BufferMemoryBarrier(_trails_vertex_buffer[current_index].buffer, 1000 * TrailRenderer::max_points * sizeof(Vertex), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    }
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, frame_overlaps, vertex_buffer_barriers, 0, nullptr);
}

void mrs::TrailRenderPipeline::Init()
{
    _scene = Application::Instance().GetScene();

    // Create trails vertex buffers
    _trails_vertex_buffer.resize(frame_overlaps);
    for (uint32_t i = 0; i < frame_overlaps; i++)
    {
        _trails_vertex_buffer[i] = _renderer->CreateBuffer(1000 * TrailRenderer::max_points * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        _renderer->DeletionQueue().Push([&](){
            vmaDestroyBuffer(_renderer->Allocator(), _trails_vertex_buffer[i].buffer, _trails_vertex_buffer[i].allocation);
        });
    }

    InitDescriptors();

    Ref<Shader> line_vertex_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/default_line_shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    PushShader(line_vertex_shader);

    Ref<Shader> line_fragment_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/default_line_shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    PushShader(line_fragment_shader);

    //_render_pipeline_settings.primitive_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    _render_pipeline_settings.primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    _render_pass = _renderer->_offscreen_render_pass;
    BuildPipeline();

    // Effect Template
    std::vector<ShaderEffect*> default_line_effects;
    default_line_effects.push_back(Effect().get());
    Ref<EffectTemplate> default_line = VulkanAssetManager::Instance().CreateEffectTemplate(default_line_effects, "default_line");
    Material::Create(default_line, Texture::Get("default"), "default_line");

    // Register Callbacks
    _scene->Registry()->on_construct<TrailRenderer>().connect<&TrailRenderPipeline::OnTrailRendererCreated>(this);
    _scene->Registry()->on_destroy<TrailRenderer>().connect<&TrailRenderPipeline::OnTrailRendererDestroyed>(this);
}

void mrs::TrailRenderPipeline::InitDescriptors()
{
    _global_data_sets.resize(frame_overlaps);
    _object_sets.resize(frame_overlaps);
    for (uint32_t i = 0; i < frame_overlaps; i++)
    {
        VkDescriptorBufferInfo global_buffer_info = {};
        global_buffer_info.buffer = _renderer->GlobalBuffers()[i].buffer;
        global_buffer_info.offset = 0;
        global_buffer_info.range = VK_WHOLE_SIZE;

        vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
            .BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build(&_global_data_sets[i], &_global_data_set_layout);

        VkDescriptorBufferInfo object_buffer_info = {};
        object_buffer_info.buffer = _renderer->ObjectBuffers()[i].buffer;
        object_buffer_info.offset = 0;
        object_buffer_info.range = VK_WHOLE_SIZE;

        vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
            .BindBuffer(0, &object_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .Build(&_object_sets[i], &_object_set_layout);
    }
}

void mrs::TrailRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
    VulkanFrameContext frame_context = _renderer->CurrentFrameData();

    // Bind global, object, and light descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_sets[current_frame], 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[current_frame], 0, nullptr);

    // Bind batch mesh vertex and index buffers
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &_trails_vertex_buffer[current_frame].buffer, &offset);

    auto& material = Material::Get("default_line");
    for (auto& entity : batch->entities)
    {
        Entity e(entity, _scene);
        auto& trail_renderer = e.GetComponent<TrailRenderer>();

        // Bind material buffer and material texures
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 2, 1, &material->DescriptorSet(), 0, nullptr);

        // Draw batch
        vkCmdDraw(cmd, trail_renderer.points.size(), 1, trail_renderer.vertex_offset, 0);
    }
}

void mrs::TrailRenderPipeline::End(VkCommandBuffer cmd) {}

void mrs::TrailRenderPipeline::UpdateDescriptors(uint32_t current_frame, float dt, RenderableBatch* batch)
{
}

void mrs::TrailRenderPipeline::OnTrailRendererCreated(entt::basic_registry<entt::entity>&, entt::entity entity)
{
    Entity e{ entity, _scene };
    e.AddComponent<Renderable>().material = mrs::Material::Get("default_line");
    auto& trail_renderer = e.GetComponent<TrailRenderer>();

    trail_renderer.last_position = e.GetComponent<mrs::Transform>().position;

    Vertex v {};
    v.position = trail_renderer.last_position;
    trail_renderer.points.push_back(v);

    trail_renderer.vertex_offset = _insert_index * sizeof(Vertex);

    // Check if using cached vertex index
    if(_insert_index < n_trails)
    {
        _insert_index = n_trails;
    }
    else
    {
        _insert_index++;
    }

    n_trails++;

    trail_renderer.dirty = true;
}

void mrs::TrailRenderPipeline::OnTrailRendererDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
{
    Entity e{ entity, _scene };
    auto& trail_renderer = e.GetComponent<TrailRenderer>();

    _insert_index  = trail_renderer.insert_index;
}

