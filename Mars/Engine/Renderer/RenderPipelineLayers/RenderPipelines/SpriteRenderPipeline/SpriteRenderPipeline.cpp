#include "SpriteRenderPipeline.h"

#include "ECS/Components/Components.h"
#include "Renderer/Vulkan/VulkanInitializers.h"
#include "Renderer/RenderPipelineLayers/IRenderPipelineLayer.h"
#include "Core/Time.h"

mrs::SpriteRenderPipeline::SpriteRenderPipeline()
    :IRenderPipeline("SpriteRenderPipeline")
{
}

mrs::SpriteRenderPipeline::~SpriteRenderPipeline() {}

void mrs::SpriteRenderPipeline::Init()
{
    InitDescriptors();

    Ref<Shader> ui_vertex_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/sprite_shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    PushShader(ui_vertex_shader);

    Ref<Shader> ui_fragment_shader = VulkanAssetManager::Instance().LoadShader("Assets/Shaders/sprite_shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    PushShader(ui_fragment_shader);

    VkPushConstantRange push_constant = {};
    push_constant.offset = 0;
    push_constant.size = sizeof(SpritePushConstant);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    _render_pipeline_settings.push_constants.push_back(push_constant);
    _render_pass = _renderer->_offscreen_render_pass;
    BuildPipeline();

    // Effect Template
    std::vector<ShaderEffect*> default_ui_effects;
    default_ui_effects.push_back(Effect().get());
    Ref<EffectTemplate> default_ui = VulkanAssetManager::Instance().CreateEffectTemplate(default_ui_effects, "default_ui");

    Material::Create(default_ui, Texture::Get("default"), "default_ui");

    // Fonts
    auto ken_pixel = Texture::LoadFromAsset("Assets/Textures/KenPixel.bp", "KenPixel");
    Material::Create(default_ui, ken_pixel, "default_ui");
}

void mrs::SpriteRenderPipeline::InitDescriptors()
{
    VkDescriptorBufferInfo global_buffer_info = {};
    global_buffer_info.buffer = _renderer->GlobalBuffer().buffer;
    global_buffer_info.offset = 0;
    global_buffer_info.range = VK_WHOLE_SIZE;

    vkutil::DescriptorBuilder::Begin(_renderer->DescriptorLayoutCache(), _renderer->DescriptorAllocator())
        .BindBuffer(0, &global_buffer_info, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(&_global_data_set, &_global_data_set_layout);

    _object_sets.resize(frame_overlaps);
    for (uint32_t i = 0; i < frame_overlaps; i++)
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

void mrs::SpriteRenderPipeline::Begin(VkCommandBuffer cmd, uint32_t current_frame, RenderableBatch* batch)
{
    UpdateDescriptors();

    uint32_t n_frame = _renderer->CurrentFrame();
    VulkanFrameContext frame_context = _renderer->CurrentFrameData();

    // Bind global, object, and light descriptors
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_global_data_set, 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1, &_object_sets[n_frame], 0, nullptr);

    for (auto& e : batch->entities)
    {
        auto& renderable = e.GetComponent<Renderable>();

        if (!renderable.enabled)
        {
            continue;
        }

        auto& sprite_renderer = e.GetComponent<SpriteRenderer>();

        const Ref<Texture>& atlas = sprite_renderer.sprite->Atlas();
        SpritePushConstant pc
        {
            {0,0, atlas->_width, atlas->_height}, // Assume we using the entire atlas
            sprite_renderer.sprite->Rect(sprite_renderer.sprite_index)
        };

        // Bind ui quad and ui material
        Ref<Mesh> mesh = sprite_renderer.mesh != nullptr? sprite_renderer.mesh : Mesh::Get("quad");
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->_buffer.buffer, &offset);

        if (mesh->_index_count > 0)
        {
            vkCmdBindIndexBuffer(cmd, mesh->_index_buffer.buffer, offset, VK_INDEX_TYPE_UINT32);
        }

        vkCmdPushConstants(cmd, _pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SpritePushConstant), &pc);

        // Bind material buffer and material texures
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 2, 1, &renderable.material->DescriptorSet(), 0, nullptr);

        // Bind Sprite Atlas
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 3, 1, &sprite_renderer.sprite->DescriptorSet(), 0, nullptr);
        vkCmdDrawIndexed(cmd, mesh->_index_count, 1, 0, 0, e.Id());
    }
}

void mrs::SpriteRenderPipeline::End(VkCommandBuffer cmd)
{
}

void mrs::SpriteRenderPipeline::UpdateDescriptors()
{
    Scene* scene = Application::Instance().GetScene();

    auto view = scene->Registry()->view<SpriteRenderer, SpriteAnimator>();

    for (auto entity : view)
    {
        Entity e(entity, scene);
        auto& sprite_renderer = e.GetComponent<SpriteRenderer>();
        auto& sprite_animator = e.GetComponent<SpriteAnimator>();

        if (!sprite_animator.playing)
        {
            continue;
        }

        sprite_animator.time_elapsed += Time::DeltaTime();
        if (sprite_animator.time_elapsed >= (1.0f / sprite_animator.fps))
        {
            sprite_renderer.sprite_index = (sprite_renderer.sprite_index + 1) % sprite_renderer.sprite->SpriteCount();
            sprite_animator.time_elapsed = 0.0f;
        }
    }
}
