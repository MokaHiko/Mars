#include "CBRenderPipeline.h"

#include <ECS/Components/Components.h>
#include <Renderer/Vulkan/VulkanInitializers.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Math/Math.h>
#include <Renderer/RenderPipelineLayers/IRenderPipelineLayer.h>

#include <Core/Time.h>

CBRenderPipeline::CBRenderPipeline(const std::string& name)
    :IRenderPipeline(name) {}

CBRenderPipeline::CBRenderPipeline(const std::string& name, VkRenderPass render_pass)
    :IRenderPipeline(name, render_pass) {}

CBRenderPipeline::~CBRenderPipeline()
{
}

void CBRenderPipeline::Init()
{
    using namespace mrs;

    // TODO: Maybe put in Space sim layer
    // Connect to RigidBody2D Component signals
    Scene* scene = Application::Instance().GetScene();
    scene->Registry()->on_construct<CelestialBody>().connect<&CBRenderPipeline::OnCelestialBodyCreated>(this);
    scene->Registry()->on_destroy<CelestialBody>().connect<&CBRenderPipeline::OnCelestialBodyDestroyed>(this);
    scene->Registry()->on_update<CelestialBody>().connect<&CBRenderPipeline::OnCelestialBodyUpdated>(this);

    // Reflection based descriptor set/layout creation
    Ref<Shader> celestial_vertex_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/cb_shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    PushShader(celestial_vertex_shader);

    Ref<Shader> celestial_tesc_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/cb_shader.tesc.spv", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    PushShader(celestial_tesc_shader);

    Ref<Shader> celestial_tese_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/cb_shader.tese.spv", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    PushShader(celestial_tese_shader);

    Ref<Shader> celestial_fragment_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/cb_shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    PushShader(celestial_fragment_shader);

    // Assign render pass
    _render_pass = _renderer->_offscreen_render_pass;

    // Configure pipeline settings
    // _render_pipeline_settings.polygon_mode = VK_POLYGON_MODE_LINE;
    _render_pipeline_settings.polygon_mode = VK_POLYGON_MODE_FILL;
    _render_pipeline_settings.primitive_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    _render_pipeline_settings.tesselation_control_points = 4;

    // Push constants
    VkPushConstantRange cb_push_constant = {};
    cb_push_constant.offset = 0;
    cb_push_constant.size = sizeof(CelestialBodyData);
    cb_push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    _render_pipeline_settings.push_constants.push_back(cb_push_constant);

    // Build Render Pipeline
    BuildPipeline();

    // Grab handle to and init descriptors used in shaders
    InitDescriptors();

    // Init Indirect drawing
    InitIndirectCommands();
}

void CBRenderPipeline::InitDescriptors()
{
    using namespace mrs;

	// TODO: Put in by default
    {
		VkDescriptorBufferInfo global_buffer_info = {};
		global_buffer_info.buffer = _renderer->GlobalBuffer().buffer;
		global_buffer_info.offset = 0;
		global_buffer_info.range = VK_WHOLE_SIZE;

		vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
			.BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
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
				.BindBuffer(0, &object_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
				.Build(&_object_sets[i], &_object_set_layout);

			VkDescriptorBufferInfo dir_light_buffer_info = {};
			dir_light_buffer_info.buffer = _renderer->DirLightBuffers()[i].buffer;
			dir_light_buffer_info.offset = 0;
			dir_light_buffer_info.range = VK_WHOLE_SIZE;

			vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
				.BindBuffer(0, &dir_light_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.Build(&_dir_light_sets[i], &_dir_light_layout);
		}
    }

    // CB Descriptors
    {
		_celestial_body_sets.resize(frame_overlaps);

		size_t noise_buffer_size = _renderer->PadToStorageBufferSize(sizeof(NoiseSettings)) * MAX_NOISE_SETTINGS;
		_noise_settings_buffers.resize(frame_overlaps);
		for (uint32_t i = 0; i < frame_overlaps; i++)
		{
			_noise_settings_buffers[i] = _renderer->CreateBuffer(noise_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

			VkDescriptorBufferInfo noise_settings_buffer_info = {};
			noise_settings_buffer_info.buffer = _noise_settings_buffers[i].buffer;
			noise_settings_buffer_info.offset = 0;
			noise_settings_buffer_info.range = VK_WHOLE_SIZE;

			vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
				.BindBuffer(0, &noise_settings_buffer_info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
				.Build(&_celestial_body_sets[i], &_celestial_body_layout);
			
			_renderer->DeletionQueue().Push([=]() {
				vmaDestroyBuffer(_renderer->Allocator(), _noise_settings_buffers[i].buffer, _noise_settings_buffers[i].allocation);
			});
		}
    }
}

void CBRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame, mrs::RenderableBatch* batch)
{
    if (batch->entities.empty())
    {
        return;
    }

    // TODO: Add on CB properties change or new celestial body
    _rerecord = true;

    if (_rerecord)
    {
        BuildBatches(cmd, batch);
        RecordIndirectcommands(cmd, batch);
        UpdateDescriptors(current_frame);

        _rerecord = false;
    }

    DrawObjects(cmd, batch);
}

void CBRenderPipeline::End(VkCommandBuffer cmd)
{
}

void CBRenderPipeline::OnMaterialsUpdate()
{
    _rerecord = true;
}

void CBRenderPipeline::OnRenderableCreated(mrs::Entity e)
{
    _rerecord = true;
}

void CBRenderPipeline::OnRenderableDestroyed(mrs::Entity e)
{
    _rerecord = true;
}

void CBRenderPipeline::OnCelestialBodyCreated(entt::basic_registry<entt::entity>&, entt::entity entity)
{
    using namespace mrs;

    Entity e{ entity, Application::Instance().GetScene() };
    auto& cb = e.GetComponent<CelestialBody>();

    // Create and upload terrain mesh
    Vector3 directions[6] =
    {
        {0,1,0},
        {0,-1,0},
        {-1,0,0},
        {1,0,0},
        {0,0,1},
        {0,0,-1},
    };

    cb.terrain_faces.resize(6);
    for (int i = 0; i < 6; i++)
    {
        cb.terrain_faces[i] = { 1, directions[i] };
        cb.terrain_faces[i].ConstructMesh();
        VulkanAssetManager::Instance().UploadMesh(cb.terrain_faces[i].Mesh());
    }

    RegisterCelestialBody(cb);
    _rerecord = true;
}

void CBRenderPipeline::OnCelestialBodyUpdated(entt::basic_registry<entt::entity> &, entt::entity entity) 
{
    using namespace mrs;

    Entity e{ entity, Application::Instance().GetScene() };
    auto& cb = e.GetComponent<CelestialBody>();
    RegisterCelestialBody(cb);
}

void CBRenderPipeline::OnCelestialBodyDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
{
    // TODO: Cache or Destroy Nosie Settings in storage

    // TODO: cache premade mesh data
    _rerecord = true;
}

void CBRenderPipeline::UpdateDescriptors(uint32_t frame_index)
{
    // Copy settings to noise settings storage buffer
    char* noise_data = nullptr;
    vmaMapMemory(_renderer->Allocator(), _noise_settings_buffers[frame_index].allocation, (void**)&noise_data);
    for(int i = 0; i < _noise_settings.size(); i++)
    {
        static size_t padded_noise_settings_size = _renderer->PadToStorageBufferSize(sizeof(NoiseSettings));
        memcpy(noise_data, &_noise_settings[i], sizeof(NoiseSettings));
        noise_data += padded_noise_settings_size;
    }
    vmaUnmapMemory(_renderer->Allocator(), _noise_settings_buffers[frame_index].allocation);
}

void CBRenderPipeline::RegisterCelestialBody(CelestialBody& cb)
{
    MRS_ASSERT(cb.noise_filters.size() <= MAX_NOISE_LAYERS, "[CBRenderPipeline]: Max noise filters exceeded!");

    // Find index of noise filter settings
    for (int i = 0; i < cb.noise_filters.size(); i++)
    {
        auto it = std::find_if(_noise_settings.begin(), _noise_settings.end(), [&](const NoiseSettings& settings) {
            return cb.noise_filters[i].noise_settings == settings;
        });

        if (it != _noise_settings.end())
        {
            cb.noise_filters[i].noise_filters_index = static_cast<int>(std::distance(_noise_settings.begin(), it));
        }
        else
        {
            _noise_settings.push_back(cb.noise_filters[i].noise_settings);
            cb.noise_filters[i].noise_filters_index = static_cast<int>(_noise_settings.size()) - 1;
        }
    }
}

std::vector<CBRenderPipeline::CBIndirectBatch> CBRenderPipeline::GetRenderablesAsBatches(mrs::RenderableBatch* batch)
{
    using namespace mrs;

    std::vector<CBIndirectBatch> batches = {};

    Material* last_material = nullptr;
    Mesh* last_mesh = nullptr;

    int batch_first = 0;

    for (auto e : batch->entities)
    {
        auto& cb = e.GetComponent<CelestialBody>();

		// TODO: Check if the same cb settings (Right now drawing each planet unbatched)
		RegisterCelestialBody(cb);

        for (auto& face : cb.terrain_faces)
        {
            MeshRenderer renderable;
            renderable.SetMesh(face.Mesh());
            renderable.SetMaterial(Material::Get("celestial_body"));

            // Check if new batch
            bool new_batch = renderable.GetMaterial().get() != last_material || renderable.GetMesh().get() != last_mesh;

            new_batch = true;

            if (new_batch)
            {
                CBIndirectBatch batch = {};
                batch.count = 1;
                batch.material = renderable.GetMaterial().get();
                batch.mesh = renderable.GetMesh().get();

                batch.cb_data.mask = 1;
                batch.cb_data.type = static_cast<uint32_t>(cb.type);
                batch.cb_data.n_filters = static_cast<int>(cb.noise_filters.size());

                for(uint32_t i = 0; i < cb.noise_filters.size(); i++)
                {
                    glm::value_ptr(batch.cb_data.noise_filters_indices)[i] = static_cast<int>(cb.noise_filters[i].noise_filters_index);
                }

                batch.cb_data.color_1 = cb.color_1;
                batch.cb_data.color_2 = cb.color_2;
                batch.cb_data.color_3 = cb.color_3;
                batch.cb_data.color_4 = cb.color_4;

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
    }

    return batches;
}

void CBRenderPipeline::InitIndirectCommands() {
    using namespace mrs;

    size_t max_indirect_commands = 1000;

    // Create indirect buffer per frame
    _indirect_buffers.resize(frame_overlaps);
    for (int i = 0; i < frame_overlaps; i++)
    {
        _indirect_buffers[i] = _renderer->CreateBuffer(
            _renderer->PadToStorageBufferSize(_renderer->PadToStorageBufferSize(sizeof(VkDrawIndexedIndirectCommand))) * max_indirect_commands,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU, 0);

        _renderer->DeletionQueue().Push([=]() {
            vmaDestroyBuffer(_renderer->Allocator(), _indirect_buffers[i].buffer,
            _indirect_buffers[i].allocation);
            });
    }
}

void CBRenderPipeline::BuildBatches(VkCommandBuffer cmd, mrs::RenderableBatch* batch)
{
    _noise_settings.clear();
    _batches.clear();
    _batches = GetRenderablesAsBatches(batch);
}

void CBRenderPipeline::RecordIndirectcommands(VkCommandBuffer cmd, mrs::RenderableBatch* batch)
{
    using namespace mrs;

    for (int i = 0; i < frame_overlaps; i++)
    {
        char* draw_commands;
        vmaMapMemory(_renderer->Allocator(), _indirect_buffers[i].allocation, (void**)&draw_commands);

        // Encode draw commands for each renderable ahead of time
        int ctr = 0;
        for (auto e : batch->entities)
        {
            const auto& cb = e.GetComponent<CelestialBody>();

            // 6 faces of cube
            for (auto& face : cb.terrain_faces)
            {
                VkDrawIndirectCommand draw_command = {};
                draw_command.instanceCount = 1;
                draw_command.firstInstance = e.Id();
                draw_command.firstVertex = 0;
                draw_command.vertexCount = face.Mesh()->_vertex_count;

                static size_t padded_draw_indirect_size = _renderer->PadToStorageBufferSize(sizeof(VkDrawIndirectCommand));
                size_t offset = ctr * padded_draw_indirect_size;
                memcpy(draw_commands + offset, &draw_command, sizeof(VkDrawIndirectCommand));
                ctr++;
            }
        }
        vmaUnmapMemory(_renderer->Allocator(), _indirect_buffers[i].allocation);
    }
}

void CBRenderPipeline::DrawObjects(VkCommandBuffer cmd, mrs::RenderableBatch* batch)
{
    using namespace mrs;

    uint32_t n_frame = _renderer->CurrentFrame();

    VulkanFrameContext frame_context = _renderer->CurrentFrameData();

    // Bind global, object, and light descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[n_frame], 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 3, 1, &_dir_light_sets[n_frame], 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 4, 1, &_celestial_body_sets[n_frame], 0, nullptr);

    for (auto& batch : _batches)
    {
        // Update cb data
        batch.cb_data.dt = Time::DeltaTime();
        vkCmdPushConstants(cmd, _pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 0, sizeof(CBRenderPipeline::CelestialBodyData), &batch.cb_data);

        // Bind material buffer and material texures
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 2, 1, &batch.material->DescriptorSet(), 0, nullptr);

        // Bind batch mesh vertex and index buffers
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &batch.mesh->_buffer.buffer, &offset);

        // Draw batch
        static uint32_t batch_stride = static_cast<uint32_t>(_renderer->PadToStorageBufferSize(sizeof(VkDrawIndirectCommand)));
        uint32_t indirect_offset = batch.first * batch_stride;
        vkCmdDrawIndirect(cmd, _indirect_buffers[n_frame].buffer, indirect_offset, batch.count, batch_stride);
    }
}

TerrainFace::TerrainFace(int resolution, mrs::Vector3 local_up)
    :_resolution(resolution), _local_up(local_up)
{
    _axis_a = mrs::Vector3(local_up.y, local_up.z, local_up.x);
    _axis_b = glm::cross(local_up, _axis_a);
}

TerrainFace::~TerrainFace()
{
}

void TerrainFace::ConstructMesh()
{
    using namespace mrs;

    // TOOD: Cache same _resolution/property

    std::string mesh_name = std::to_string(rand() % 1000);
    _mesh = Mesh::Create(mesh_name);

    // Generate terrain vertices
    float width = static_cast<float>(1);
    float height = static_cast<float>(1);

    for (int i = 0; i < _resolution; i++) {
        for (int j = 0; j < _resolution; j++) {
            Vertex vertex = {};
            vertex.normal = _local_up;

            vertex.position = Vector3{ 0 };
            vertex.position += _axis_a * (-width / 2.0f + (width * i / static_cast<float>(_resolution)));
            vertex.position += _axis_b * (-height / 2.0f + (height * j / static_cast<float>(_resolution)));
            vertex.position += _local_up * 0.5f;
            vertex.uv = Vector2
            {
                i / static_cast<float>(_resolution),
                j / static_cast<float>(_resolution)
            };
            _mesh->_vertices.push_back(vertex);

            vertex.position = Vector3{ 0 };
            vertex.position += _axis_a * (-width / 2.0f + (width * (i + 1) / static_cast<float>(_resolution)));
            vertex.position += _axis_b * (-height / 2.0f + (height * j / static_cast<float>(_resolution)));
            vertex.position += _local_up * 0.5f;
            vertex.uv = Vector2
            {
                (i + 1) / static_cast<float>(_resolution),
                j / static_cast<float>(_resolution)
            };
            _mesh->_vertices.push_back(vertex);

            vertex.position = Vector3{ 0 };
            vertex.position += _axis_a * (-width / 2.0f + (width * i / static_cast<float>(_resolution)));
            vertex.position += _axis_b * (-height / 2.0f + (height * (j + 1) / static_cast<float>(_resolution)));
            vertex.position += _local_up * 0.5f;
            vertex.uv = Vector2
            {
                i / static_cast<float>(_resolution),
                (j + 1) / static_cast<float>(_resolution)
            };
            _mesh->_vertices.push_back(vertex);

            vertex.position = Vector3{ 0 };
            vertex.position += _axis_a * (-width / 2.0f + (width * (i + 1) / static_cast<float>(_resolution)));
            vertex.position += _axis_b * (-height / 2.0f + (height * (j + 1) / static_cast<float>(_resolution)));
            vertex.position += _local_up * 0.5f;
            vertex.uv = Vector2
            {
                (i + 1) / static_cast<float>(_resolution),
                (j + 1) / static_cast<float>(_resolution)
            };
            _mesh->_vertices.push_back(vertex);
        }
    }

    _mesh->_vertex_count = static_cast<uint32_t>(_mesh->_vertices.size());
}

void CelestialBody::PushFilter(const NoiseFilter& filter)
{
    noise_filters.push_back(filter);
}
    
