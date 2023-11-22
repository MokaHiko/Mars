#include "GameLayer.h"

#include "Scripts/Unit.h"
#include "Scripts/Spawner.h"
#include "Scripts/GameManager.h"
#include "Scripts/Player.h"
#include "Scripts/Ship.h"
#include "Scripts/GameCamera.h"

#include <Panels/Components/ComponentPanels.h>
#include <Panels/InspectorPanel.h>

#include <glm/gtc/type_ptr.hpp>
#include "GameLayer/RenderPipelines/CBRenderPipeline.h"

// Custom Inspector Component
namespace mrs {
    template<>
    void DrawComponent<ShipResources>(Entity entity)
    {
        DrawComponentUI<ShipResources>("Ship Resources", entity, [](ShipResources& resources) 
		{
			//ImGui::DragFloat("Health", &ship._health);

			ImGui::DragInt("Credits", (int*)&resources.credit);
			ImGui::DragInt("Scrap", (int*)&resources.scrap_metal);
        });
    }

    template<>
    void DrawComponent<CelestialBody>(Entity entity)
    {
        DrawComponentUI<CelestialBody>("Celestial Body", entity, [](CelestialBody& cb) 
		{
            ImGui::DragFloat4("Color 1", glm::value_ptr(cb.color_1), 0.01, 0, 1);
            ImGui::DragFloat4("Color 2", glm::value_ptr(cb.color_2), 0.01, 0, 1);
            ImGui::DragFloat4("Color 3", glm::value_ptr(cb.color_3), 0.01, 0, 1);
            ImGui::DragFloat4("Color 4", glm::value_ptr(cb.color_4), 0.01, 0, 1);

            int ctr = 0;
            for (auto& filter : cb.noise_filters)
            {
                static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
                ImGuiTreeNodeFlags node_flags = base_flags;

                bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)(ctr++), node_flags, "Noise Filter: %d", ctr++);
                if (node_open)
                {
                    ImGui::DragInt("Layers", (int*)&filter.noise_settings.n_layers, 1.0, 0.0f, 1000.0f);
                    ImGui::DragFloat("Persistence", &filter.noise_settings.persistence, 0.01f, 0.0f, 1000.0f);
                    ImGui::DragFloat("Base Roughness", &filter.noise_settings.base_roughness, 0.01f, 0.0f, 1000.0f);
                    ImGui::DragFloat("Roughness", &filter.noise_settings.roughness, 0.01f, 0.0f, 1000.0f);

                    ImGui::DragInt("Min Resolution", (int*)&filter.noise_settings.min_resolution, 1.0, 0.0f, 1000.0f);
                    ImGui::DragInt("Max Resolution", (int*)&filter.noise_settings.max_resolution, 1.0, 0.0f, 1000.0f);

                    ImGui::DragFloat("Strength", &filter.noise_settings.strength, 0.01f, 0.0f, 1000.0f);
                    ImGui::DragFloat("MinValue", &filter.noise_settings.min_value, 0.01f, 0.0f, 1000.0f);
                    ImGui::DragFloat4("Center", glm::value_ptr(filter.noise_settings.center), 0.1f, 0.0f, 1000.0f);

                    ImGui::TreePop();
                }
            }
        });
    }
}

void GameLayer::OnAttach()
{
	RegisterScripts();

    mrs::InspectorPanel::AddCustomComponentInspector<CelestialBody>();
    mrs::InspectorPanel::AddCustomComponentInspector<ShipResources>();
};

void GameLayer::RegisterScripts()
{
	mrs::Script::Register<Unit>();
	mrs::Script::Register<Spawner>();
	mrs::Script::Register<GameManager>();
	mrs::Script::Register<GameCamera>();
	mrs::Script::Register<Player>();
	mrs::Script::Register<Ship>();
}

void GameLayer::OnDetatch() {};
void GameLayer::OnEnable() {};
void GameLayer::OnDisable() {};

void GameLayer::OnUpdate(float dt) {};

void GameLayer::OnImGuiRender() {};
void GameLayer::OnEvent(mrs::Event& event) {};

