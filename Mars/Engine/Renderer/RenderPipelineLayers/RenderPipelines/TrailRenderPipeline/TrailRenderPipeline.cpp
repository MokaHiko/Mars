#include "TrailRenderPipeline.h"

#include "Core/Application.h"
#include "Trails.h"

#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"

mrs::TrailRenderPipeline::TrailRenderPipeline() 
    :IRenderPipeline("TrailRenderPipeline"){}

mrs::TrailRenderPipeline::~TrailRenderPipeline() {}

void mrs::TrailRenderPipeline::OnPreRenderPass(VkCommandBuffer cmd, RenderableBatch* batch)
{
	for(auto entity : batch->entities)
	{
		Entity e(entity, _scene);
		auto& transform = e.GetComponent<mrs::Transform>();
		auto& trail_renderer = e.GetComponent<TrailRenderer>();

		Vertex v = {};
		v.position = transform.position;
		trail_renderer.mesh->Vertices()[1] = v;
		size_t buffer_size = sizeof(Vertex) * trail_renderer.mesh->Vertices().size();
		vkCmdUpdateBuffer(cmd, trail_renderer.mesh->_buffer.buffer, 0, buffer_size, trail_renderer.mesh->Vertices().data());

        // Vector3 last_point = {};
        // if (trail_renderer.points.size() > 0)
        // {
        //     last_point = trail_renderer.points.back();
        // }

		// if(glm::length(last_point -  transform.position) >= trail_renderer.min_vertex_distance)
		// {
		// 	trail_renderer.points.push_back(transform.position);

		// 	// Update mesh vbo
		// 	Vertex v = {};
		// 	v.position = transform.position;

		// 	trail_renderer.mesh->Vertices().push_back(v);

		// 	// Update mesh

		// 	//vkCmdUpdateBuffer(cmd, trail_renderer.mesh->_buffer.buffer, 0, buffer_size, trail_renderer.mesh->Vertices().data());

		// 	MRS_INFO("VBO Updated!");
		// 	// Pipeline Barrier
		// }
	}
}

void mrs::TrailRenderPipeline::Init() 
{
	_scene = Application::Instance().GetScene();
	const uint32_t max_trail_points = sizeof(Vertex) / 65536;

    InitDescriptors();

    Ref<Shader> line_vertex_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/default_line_shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    PushShader(line_vertex_shader);

    Ref<Shader> line_fragment_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/default_line_shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    PushShader(line_fragment_shader);

	_render_pipeline_settings.primitive_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

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

void mrs::TrailRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
    VulkanFrameContext frame_context = _renderer->CurrentFrameData();

    // Bind global, object, and light descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[current_frame], 0, nullptr);

	// TODO: Indirect Draw in batches
	auto& material = Material::Get("default_line");
    for (auto& entity : batch->entities)
    {
		Entity e(entity, _scene);
		auto& trail_renderer = e.GetComponent<TrailRenderer>();

        // Bind material buffer and material texures
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 2, 1, &material->DescriptorSet(), 0, nullptr);

        // Bind batch mesh vertex and index buffers
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &trail_renderer.mesh->_buffer.buffer, &offset);

        if (trail_renderer.mesh->Indices().size() > 0)
        {
            vkCmdBindIndexBuffer(cmd, trail_renderer.mesh->_index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        }

        // Draw batch
        static uint32_t batch_stride = static_cast<uint32_t>(_renderer->PadToStorageBufferSize(sizeof(VkDrawIndexedIndirectCommand)));
        vkCmdDrawIndexed(cmd, trail_renderer.mesh->Indices().size(), 1, 0, 0, e.Id());
    }
}

void mrs::TrailRenderPipeline::End(VkCommandBuffer cmd) {}

void mrs::TrailRenderPipeline::OnTrailRendererCreated(entt::basic_registry<entt::entity> &, entt::entity entity) 
{
    Entity e{ entity, _scene };
	e.AddComponent<Renderable>().material = mrs::Material::Get("default_line");
    auto& trail_renderer = e.GetComponent<TrailRenderer>();

    if(!trail_renderer.mesh)
    {
        std::string trail_mesh_name = "trail_" + std::to_string(e.Id());
        trail_renderer.mesh = Mesh::Create(trail_mesh_name);

        Vertex v = {};
        v.position = e.GetComponent<mrs::Transform>().position;
        trail_renderer.mesh->Vertices() = {v, {}};
        trail_renderer.mesh->Indices() = {0,1};

        VulkanAssetManager::Instance().UploadMesh(trail_renderer.mesh);
    }
}

void mrs::TrailRenderPipeline::OnTrailRendererDestroyed(entt::basic_registry<entt::entity> &, entt::entity entity) 
{

}

void mrs::TrailRenderPipeline::UpdateTrails() 
{
}
