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
	void DrawComponent<Serializer>(Entity entity)
	{
		DrawComponentUI<Serializer>("Serializer", entity, [](Serializer& serializer) {
			if (ImGui::Checkbox("Serialize", &serializer.serialize))
			{
			}
			});
	}

	template<>
	void DrawComponent<Transform>(Entity entity)
	{
		DrawComponentUI<Transform>("Transform", entity, [](Transform& transform) {
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
	void DrawComponent<MeshRenderer>(Entity entity)
	{
		DrawComponentUI<MeshRenderer>("Mesh Renderer", entity, [](MeshRenderer& renderable)
			{
				ImGui::Text("Mesh: %s", renderable.GetMesh()->_mesh_name.c_str());
				ImGui::Text("Vertices: %d", renderable.GetMesh()->_vertex_count);
				ImGui::Text("Indices: %d", renderable.GetMesh()->_index_count);
				ImVec2 region_size = ImGui::GetContentRegionAvail();

				Ref<Material> material = renderable.GetMaterial();
				ImGui::Text("material: %s", material->Name().c_str());
				ImGui::Text("diffuse: %s", material->GetTexture(mrs::MaterialTextureType::DiffuseTexture)->_name.c_str());
				ImGui::Text("specular: %s", material->GetTexture(mrs::MaterialTextureType::SpecularTexture)->_name.c_str());
			});
	}

	template<>
	void DrawComponent<DirectionalLight>(Entity entity)
	{
		DrawComponentUI<DirectionalLight>("Directional Light", entity, [](DirectionalLight& dir_light) {
			ImGui::DragFloat4("Ambient: ", glm::value_ptr(dir_light.Ambient));
			ImGui::DragFloat4("Diffuse: ", glm::value_ptr(dir_light.Diffuse));
			ImGui::DragFloat4("Specular: ", glm::value_ptr(dir_light.Specular));

			static bool view_dir_light_camera = false;
			if (ImGui::Checkbox("Set Channel", &view_dir_light_camera))
			{
			}
			});
	}

	template<>
	void DrawComponent<Script>(Entity entity)
	{
		DrawComponentUI<Script>("Script", entity, [](Script& script)
			{
				ImGui::Text(script.binding.c_str());
				ImGui::Checkbox("Enabled", &script.enabled);
			});
	}

	template<>
	void DrawComponent<Camera>(Entity entity)
	{
		DrawComponentUI<Camera>("Camera", entity, [](Camera& camera)
			{
				CameraType camera_type = camera.GetType();
				bool is_orthographic = CameraType::Orthographic == camera_type;
				bool is_perspective = !is_orthographic;

				ImGui::Text("Aspect W: "); ImGui::SameLine();
				ImGui::DragInt("##AspectW", (int*)(&camera._aspect_w));

				ImGui::Text("Aspect H: "); ImGui::SameLine();
				ImGui::DragInt("##AspectH", (int*)(&camera._aspect_h));

				ImGui::Text("Near: "); ImGui::SameLine();
				ImGui::DragFloat("##Near", (float*)(&camera._near));

				ImGui::Text("Far: "); ImGui::SameLine();
				ImGui::DragFloat("##Far", (float*)(&camera._far));

				ImGui::Text("Yaw: "); ImGui::SameLine();
				ImGui::DragFloat("##Yaw", (float*)(&camera.Yaw()));

				ImGui::Text("Pitch: "); ImGui::SameLine();
				ImGui::DragFloat("##Pitch", (float*)(&camera.Pitch()));

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
		static char* body_types[] = { "UNKNOWN", "STATIC", "DYNAMIC" };
		DrawComponentUI<RigidBody2D>("RigidBody2D", entity, [](RigidBody2D& rb)
			{
				ImGui::Checkbox("Use Gravity", &rb.use_gravity);
				ImGui::Text("Body Type: %s", body_types[(int)rb.type]);
			});
	}

	template<>
	void DrawComponent<MeshCollider>(Entity entity)
	{
		static char* collider_types[] =
		{
			"UKNOWN",
			"SPHERE_COLLIDER",
			"PLANE_COLLIDER"
		};

		DrawComponentUI<MeshCollider>("Mesh Collider", entity, [](MeshCollider& col) {
			uint8_t type_index = static_cast<uint8_t>(col.type);
			ImGui::Text("Type: %s", collider_types[type_index]);
			});
	}

	template<>
	void DrawComponent<ParticleSystem>(Entity entity)
	{
		DrawComponentUI<ParticleSystem>("Particle System", entity, [](ParticleSystem& particles)
			{
				// Emission properties
				ImGui::InputInt("Max Particles: ", (int*)(&particles.max_particles));
				ImGui::DragInt("Live Particles", (int*)(&particles.live_particles));
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
					const char* items[] = { "None", "Circle", "Cone" };
					int item_current = static_cast<int>(particles.emission_shape);
					ImGui::Combo("Emission Shape", &item_current, items, IM_ARRAYSIZE(items));
					ImGui::SameLine();

					ImGui::TreePop();
				}
			});
	}
};