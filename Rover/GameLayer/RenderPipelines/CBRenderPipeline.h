#ifndef CBRENDERPIPELINE_H
#define CBRENDERPIPELINE_H

#pragma once

#include <Renderer/RenderPipelineLayers/IRenderPipelineLayer.h>
#include <Math/Math.h>

class TerrainFace
{
public:
    TerrainFace() = default;
    TerrainFace(int resolution, mrs::Vector3 local_up);
    ~TerrainFace();

    void ConstructMesh();

    Ref<mrs::Mesh> Mesh() const { return _mesh; }
private:
    Ref<mrs::Mesh> _mesh = nullptr;
    int _resolution = 1;

    mrs::Vector3 _local_up{ 0, 1, 0 };

    mrs::Vector3 _axis_a = {}; // right vector
    mrs::Vector3 _axis_b = {}; // forward vector
};

struct NoiseSettings
{
    uint32_t n_layers = 1;
    float persistence = 0.5f;

    float base_roughness = 1.0f;
    float roughness = 2.0f;

    uint32_t min_resolution = 16;
    uint32_t max_resolution = 16;

    float strength = 0.25;
    float min_value = 0.25;

    mrs::Vector4 center = { 0,0,0,0 };

    bool operator==(const NoiseSettings& other) const
    {
        if (n_layers == other.n_layers) return false;
        if (persistence == other.persistence) return false;
        if (base_roughness == other.roughness) return false;
        if (min_resolution == other.max_resolution) return false;
        if (strength == other.strength) return false;
        if (min_value == other.min_value) return false;
        if (center == other.center) return false;

        return true;
    }
};

struct NoiseFilter
{
    bool enabled = true;

    NoiseSettings noise_settings = {};
    // Index of noise filter in storage buffer
    int noise_filters_index = 0;
};

struct CelestialBody
{
public:
    enum class Type
    {
        Uknown,
        Planet,
        Moon,
        Star,
    };

    Type type = {};
    void PushFilter(const NoiseFilter& filter);

    std::vector<NoiseFilter> noise_filters = {}; // Maximuim of 4 noise filters for now
    std::vector<TerrainFace> terrain_faces = {};

    mrs::Vector4 color_1{0,0.250,1.0, 0.0f};
    mrs::Vector4 color_2{0.488f,0.299f,0.120,0.1};
    mrs::Vector4 color_3{0,0.410, 0, 0.16};
    mrs::Vector4 color_4{0.75, 0.75,0.75, 0.35};
};

// Render Pipeline for procedurally generating and rendering celestial bodies i.e Planets, Moons, Stars
class CBRenderPipeline : public mrs::IRenderPipeline
{
public:
    CBRenderPipeline(const std::string& name = "CBRenderPipeline");
    CBRenderPipeline(const std::string& name, VkRenderPass render_pass);
    ~CBRenderPipeline();

    virtual void Init() override;
    virtual void InitDescriptors() override;

    virtual void Begin(VkCommandBuffer cmd, uint32_t current_frame, mrs::RenderableBatch* batch) override;
    virtual void End(VkCommandBuffer cmd) override;

    virtual void OnMaterialsUpdate() override;

    // TODO: Change Irendering layer to use Renderable Component 
    virtual void OnRenderableCreated(mrs::Entity e) override;
    virtual void OnRenderableDestroyed(mrs::Entity e) override;

    void OnCelestialBodyCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
    void OnCelestialBodyUpdated(entt::basic_registry<entt::entity>&, entt::entity entity);
    void OnCelestialBodyDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);

    void UpdateDescriptors(uint32_t frame_index);
    void RegisterCelestialBody(CelestialBody& cb);
private:
    // Celstial Bodies
    struct CelestialBodyData
    {
        float dt;
        float mask; // TODO: Change to something useful
        uint32_t n_filters; // For now only 4 max noise filters
        uint32_t type;

        mrs::Vector4Int noise_filters_indices;

        // Gradient with w component being offset in gradient
        mrs::Vector4 color_1;
        mrs::Vector4 color_2;
        mrs::Vector4 color_3;
        mrs::Vector4 color_4;
    };

    const int MAX_NOISE_LAYERS = 4;
    const int MAX_NOISE_SETTINGS = 100;

    std::vector<AllocatedBuffer> _noise_settings_buffers = {};
    std::vector<VkDescriptorSet> _celestial_body_sets = {};
    VkDescriptorSetLayout _celestial_body_layout = VK_NULL_HANDLE;

    std::vector<NoiseSettings> _noise_settings; // Map between the index in the noise settings storage buffer
private:
    // Mesh
    VkDescriptorSet _global_data_set = VK_NULL_HANDLE;
    VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

    VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> _object_sets = {};

    VkDescriptorSetLayout _dir_light_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet>  _dir_light_sets = {};
private:
    struct CBIndirectBatch
    {
        mrs::Mesh* mesh;
        mrs::Material* material;
        CelestialBodyData cb_data;

        uint32_t first; // batches first instance in draw indirect buffer
        uint32_t count; // batch member count
    };

    // Indirect Drawing
    void InitIndirectCommands();

    void BuildBatches(VkCommandBuffer cmd, mrs::RenderableBatch* batch);
    void RecordIndirectcommands(VkCommandBuffer cmd, mrs::RenderableBatch* batch);

    void DrawObjects(VkCommandBuffer cmd, mrs::RenderableBatch* batch);

    // Returns vector of indirect draw batches from renderable batch
    std::vector<CBIndirectBatch> GetRenderablesAsBatches(mrs::RenderableBatch* batch);

    // Flag set when draw commands need to be updated i.e Entity creation and destruction
    bool _rerecord = true;

    std::vector<AllocatedBuffer> _indirect_buffers;
    std::vector<CBIndirectBatch> _batches = {};

};

#endif