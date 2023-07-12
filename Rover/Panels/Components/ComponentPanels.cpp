#include "ComponentPanels.h"

#include <ECS/Components/Components.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <Renderer/RenderPipelineLayers/RenderPipelines/ParticleRenderPipeline/ParticleComponents.h>
#include <Core/Application.h>

namespace mrs
{
	template<>
	void DrawComponent<Transform>(Entity entity)
	{
		DrawComponentUI<Transform>("Transform", entity, [](Transform &transform) {
			ImGui::BeginTable("TransformTable", 2);

			float label_width = ImGui::GetContentRegionAvail().x * 0.25f;
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, label_width);
			ImGui::TableSetupColumn("Vec3 Value", ImGuiTableColumnFlags_WidthStretch, label_width * 3.0f);

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Position: ");


			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(label_width * 3.0f);
			ImGui::DragFloat3("##Position", glm::value_ptr(transform.position));
			ImGui::PopItemWidth();

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Rotation: ");

			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(label_width * 3.0f);
			ImGui::DragFloat3("##Rotation", glm::value_ptr(transform.rotation));
			ImGui::PopItemWidth();

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scale: ");

			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(label_width * 3.0f);
			ImGui::DragFloat3("##Scale", glm::value_ptr(transform.scale));
			ImGui::PopItemWidth();

			ImGui::EndTable();
			});
	}

	template<>
	void DrawComponent<RenderableObject>(Entity entity)
	{
		DrawComponentUI<RenderableObject>("Renderable Object", entity, [](RenderableObject &renderable) {
			ImGui::Text("Mesh: %s", renderable.mesh->_mesh_name.c_str());

			ImVec2 region_size = ImGui::GetContentRegionAvail();
			ImGui::Text("Diffuse"); ImGui::SameLine();
			ImGui::Text("Specular"); ImGui::SameLine();
			ImGui::Text("Emissive");
			ImGui::Image(renderable.material->texture_set, { region_size.x / 3.0f, region_size.y / 5.0f }); ImGui::SameLine();
			ImGui::Image(renderable.material->texture_set, { region_size.x / 3.0f, region_size.y / 5.0f }); ImGui::SameLine();
			ImGui::Image(renderable.material->texture_set, { region_size.x / 3.0f, region_size.y / 5.0f });
			});
	}

	template<>
	void DrawComponent<Script>(Entity entity)
	{
		DrawComponentUI<Script>("Script", entity, [](Script &script) {
			ImGui::Button(script.binding.c_str());
			});
	}

	template<>
	void DrawComponent<Camera>(Entity entity)
	{
		DrawComponentUI<Camera>("Camera", entity, [](Camera &camera) {
			CameraType camera_type = camera.GetType();
			bool is_orthographic = CameraType::Orthographic == camera_type;
			bool is_perspective = !is_orthographic;

			ImGui::Text("Aspect W: "); ImGui::SameLine();
			ImGui::DragInt("##AspectW", (int *)(&camera._aspect_w));

			ImGui::Text("Aspect H: "); ImGui::SameLine();
			ImGui::DragInt("##AspectH", (int *)(&camera._aspect_h));

			ImGui::Text("Near: "); ImGui::SameLine();
			ImGui::DragFloat("##Near", (float *)(&camera._near));

			ImGui::Text("Far: "); ImGui::SameLine();
			ImGui::DragFloat("##Far", (float *)(&camera._far));

			if (ImGui::Checkbox("Orthographic", &is_orthographic))
			{
				camera.SetType(CameraType::Orthographic);
			}

			ImGui::SameLine();

			if (ImGui::Checkbox("Perspective", &is_perspective))
			{
				camera.SetType(CameraType::Perspective);
			}

			ImGui::Checkbox("active", &camera._active);
			});
	}

	template<>
	void DrawComponent<RigidBody2D>(Entity entity)
	{
		static char *body_types[] = { "UNKNOWN", "STATIC", "DYNAMIC"};
		DrawComponentUI<RigidBody2D>("RigidBody2D", entity, [](RigidBody2D &rb) {
			ImGui::Checkbox("Use Gravity", &rb.use_gravity);
			ImGui::Text("Body Type: %s", body_types[(int)rb.type]);
			});
	}

	template<>
	void DrawComponent<ParticleSystem>(Entity entity)
	{
		DrawComponentUI<ParticleSystem>("Particle System", entity, [](ParticleSystem &particles) {
			// Emission properties
			ImGui::InputInt("Max Particles: ", (int *)(&particles.max_particles));
			ImGui::DragInt("Live Particles", (int *)(&particles.live_particles));
			ImGui::DragFloat("Emission Rate: ", (&particles.emission_rate), 0.5f, 0.1f, 512.0f);

			ImGui::Checkbox("Repeating", &particles.repeating);
			ImGui::Checkbox("Running", &particles.running);

			if (ButtonCentered("Reset"))
			{
				particles.Reset();
			}

			// Particle properties
			if (ImGui::TreeNode("Particle Properties"))
			{
				ImGui::DragFloat("Lifetime", &particles.life_time, 0.1f, 0.0f, std::numeric_limits<float>::max());
				ImGui::DragFloat("Scale", &particles.particle_size, 0.5f, 0.0f, std::numeric_limits<float>::max());

				ImGui::Text("Color Gradient");
				ImGui::ColorEdit4("color 1", glm::value_ptr(particles.color_1));
				ImGui::ColorEdit4("color 2", glm::value_ptr(particles.color_2));

				ImGui::DragFloat2("Starting Velocity", glm::value_ptr(particles.velocity), 0.5f, 0.0f, 10000.0f);

				ImGui::TreePop();
			}

			// Shape
			if (ImGui::TreeNode("Shape"))
			{
				const char *items[] = { "None", "Circle", "Cone" };
				int item_current = static_cast<int>(particles.emission_shape);
				ImGui::Combo("Emission Shape", &item_current, items, IM_ARRAYSIZE(items));
				ImGui::SameLine();

				ImGui::TreePop();
			}
			});
	}
};