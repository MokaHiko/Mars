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

    Ref<mrs::Mesh> Mesh () const {return _mesh;}
private:
    Ref<mrs::Mesh> _mesh = nullptr;
    int _resolution = 1;

    mrs::Vector3 _local_up{0, 1, 0};

    mrs::Vector3 _axis_a = {}; // right vector
    mrs::Vector3 _axis_b = {}; // forward vector
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

    const Type CelestialType() const {return _type;}
    std::vector<TerrainFace> terrain_faces = {};
public:
    int _radius = 1;
    uint32_t _min_resolution = 16;
    uint32_t _max_resolution = 16;
    float _strength = 0.25;
    float _roughness = 1.0f;

    mrs::Vector4 center = { 0,0,0,0 };
    Type _type = {};
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

    // TODO: Remove
    virtual void OnRenderableCreated(mrs::Entity e) override;
    virtual void OnRenderableDestroyed(mrs::Entity e) override;

private:
    void OnCelestialBodyCreated(entt::basic_registry<entt::entity>&, entt::entity entity);
    void OnCelestialBodyDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity);

    struct CelstialBodyData
    {
        uint32_t resolution = 16;
        float radius = 1;
        float strength = 1;
        float roughness = 1;
        float dt = 0;

        mrs::Vector4 center;
    };

    std::vector<AllocatedBuffer> _celestial_body_buffers = {};
    std::vector<VkDescriptorSet> _celestial_body_sets = {};
private:
    // Mesh
    VkDescriptorSet _global_data_set = VK_NULL_HANDLE;
    VkDescriptorSetLayout _global_data_set_layout = VK_NULL_HANDLE;

    VkDescriptorSetLayout _object_set_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> _object_sets = {};

    VkDescriptorSetLayout _dir_light_layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet>  _dir_light_sets = {};
private:
    // Indirect Drawing
    void InitIndirectCommands();

    void BuildBatches(VkCommandBuffer cmd, mrs::RenderableBatch* batch);
    void RecordIndirectcommands(VkCommandBuffer cmd, mrs::RenderableBatch* batch);

    void DrawObjects(VkCommandBuffer cmd, mrs::RenderableBatch* batch);

    // Returns vector of indirect draw batches from renderable batch
    std::vector<mrs::IndirectBatch> GetRenderablesAsBatches(mrs::RenderableBatch* batch);

    // Flag set when draw commands need to be updated i.e Entity creation and destruction
    bool _rerecord = true;

    std::vector<AllocatedBuffer> _indirect_buffers;
    std::vector<mrs::IndirectBatch> _batches = {};
    std::vector<CelestialBody> _celestial_bodies = {};
};

#endif