#include "InspectorPanel.h"

std::vector<std::function<void(mrs::Entity)>> mrs::InspectorPanel::_custom_component_fns = {};

void mrs::InspectorPanel::Draw(Entity entity)
{
	ImGui::Begin("Inspector");
	if (entity.IsAlive())
	{
		Tag& tag = entity.GetComponent<Tag>();

		//ImGui::InputText("##label", &tag.tag);
		ImGui::Text("Id: %d Tag: %s", entity.Id(), tag.tag.c_str());

		ImGui::Separator();

		DrawComponent<Transform>(entity);
		DrawComponent<MeshRenderer>(entity);
		DrawComponent<Camera>(entity);
		DrawComponent<Script>(entity);
		DrawComponent<RigidBody2D>(entity);
		DrawComponent<ParticleSystem>(entity);
		DrawComponent<DirectionalLight>(entity);
		DrawComponent<MeshCollider>(entity);
		DrawComponent<SpriteRenderer>(entity);
		DrawComponent<SpriteAnimator>(entity);

		for(auto& fn : _custom_component_fns)
		{
			fn(entity);
		}

#ifdef MRS_DEBUG
		DrawComponent<Serializer>(entity);
#endif
		DrawAddComponent(entity);
	}

	ImGui::End();
}

void mrs::InspectorPanel::DrawAddComponent(Entity entity)
{
	static bool searching = false;
	ImGui::Separator();
	if (ImGui::Selectable("+ Add Component"))
	{
		searching = true;
	}

	if (searching)
	{
		static ImGuiTextFilter filter;
		filter.Draw();
		static std::vector<const char*> components = [&]() {
			std::vector<const char*> comp = { "Transform", "Renderable", "Camera", "RigidBody2D", "ParticleSystem" };
			for (auto it = Script::script_instantiation_bindings.begin(); it != Script::script_instantiation_bindings.end(); it++)
			{
				comp.push_back(it->first.c_str());
			}
			return comp;
			}();

			for (int i = 0; i < components.size(); i++)
			{
				if (filter.PassFilter(components[i]))
				{
					if (ImGui::Selectable(components[i]))
					{
						switch (i)
						{
						case 0:
							entity.AddComponent<Transform>();
							break;
						case 1:
							entity.AddComponent<MeshRenderer>();
							break;
						case 2:
							entity.AddComponent<Camera>();
							break;
						case 3:
							entity.AddComponent<RigidBody2D>();
							break;
						case 4:
							entity.AddComponent<ParticleSystem>();
							break;
						default:
						{
							auto& script = entity.AddComponent<Script>();
							script.Bind(components[i], entity);
							script.enabled = false;
						} break;
						}
						searching = false;
					}
				}
			}
	}
}

