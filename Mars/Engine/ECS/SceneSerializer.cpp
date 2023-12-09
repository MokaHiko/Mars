#include "SceneSerializer.h"
#include "Entity.h"

#include "Toolbox/RandomToolBox.h"
#include "Components/Components.h"
#include "Physics/Physics.h"
#include <fstream>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		};

		static bool decode(const Node &node, glm::vec2 &rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		};
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		};

		static bool decode(const Node &node, glm::vec3 &rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();

			return true;
		};
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		};

		static bool decode(const Node &node, glm::vec4 &rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();


			return true;
		};
	};

	Emitter &operator<<(Emitter &out, const glm::vec2 &v)

	{
		out << Flow;
		out << BeginSeq << v.x << v.y << EndSeq;
		return out;
	}
	Emitter &operator<<(Emitter &out, const glm::vec3 &v)

	{
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << EndSeq;
		return out;
	}

	Emitter &operator<<(Emitter &out, const glm::vec4 &v)
	{
		out << Flow;
		out << BeginSeq << v.x << v.y << v.z << v.w << EndSeq;
		return out;
	}
}

mrs::SceneSerializer::SceneSerializer(Scene *scene)
	:_scene(scene)
{
}

mrs::SceneSerializer::~SceneSerializer() {}

void mrs::SceneSerializer::SerializeText(const std::string &scene_name, const std::string &out_path)
{
	YAML::Emitter out = {};
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << scene_name.c_str();
	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

	auto view = _scene->Registry()->view<Transform, Serializer>();
	for(auto e : view) 
	{
		Entity entity = { e, _scene };

		if (!entity)
		{
			continue;
		}

		if(!entity.GetComponent<Serializer>().serialize)
		{
			continue;
		}

		SerializeEntity(out, entity);
	};
	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::string file_path = out_path + "/" + scene_name + ".yaml";
	std::ofstream fout(file_path);
	if (fout.is_open())
	{
		fout << out.c_str();
		fout.close();
	}
}

bool mrs::SceneSerializer::DeserializeText(const std::string &scene_path)
{
	std::ifstream stream(scene_path);
	std::stringstream str_stream;
	str_stream << stream.rdbuf();

	YAML::Node data = YAML::Load(str_stream.str());
	if (!data["Scene"])
	{
		return false;
	}

	std::string scene_name = data["Scene"].as<std::string>();
	std::string msg = "Deserializing scene: " + scene_name;
	MRS_TRACE(msg.c_str());

	auto entities = data["Entities"];
	if (!entities)
	{
		return false;
	}

	for (auto entity : entities)
	{
		auto tag_node = entity["Tag"];
		Entity new_entity = _scene->Instantiate(tag_node["Tag"].as<std::string>());

		auto transform_node = entity["Transform"];
		if (transform_node)
		{
			auto &transform = new_entity.GetComponent<Transform>();
			transform.position = transform_node["Position"].as<glm::vec3>();
			transform.rotation = transform_node["Rotation"].as<glm::vec3>();
			transform.scale = transform_node["Scale"].as<glm::vec3>();
		}

		auto renderable_node = entity["MeshRenderer"];
		if (renderable_node)
		{
			auto &renderable_object = new_entity.AddComponent<MeshRenderer>();
			renderable_object.SetMesh(Mesh::Get(renderable_node["Mesh"].as<std::string>()));
			renderable_object.SetMaterial(Material::Get(renderable_node["Material"].as<std::string>()));
		}

		auto model_node = entity["ModelRenderer"];
		if (model_node)
		{
			auto &renderable_object = new_entity.AddComponent<ModelRenderer>(Model::Get(model_node["Model"].as<std::string>()));
		}

		auto camera_node = entity["Camera"];
		if (camera_node)
		{
			auto &transform = new_entity.GetComponent<Transform>();

			int type = camera_node["Type"].as<int>();
			uint32_t aspect_w = static_cast<uint32_t>(camera_node["AspectW"].as<int>());
			uint32_t aspect_h = static_cast<uint32_t>(camera_node["AspectH"].as<int>());
			bool active = static_cast<uint32_t>(camera_node["Active"].as<bool>());

			auto &camera = new_entity.AddComponent<Camera>((CameraType)type, aspect_w, aspect_h, transform.position);
			camera.Front() = camera_node["Front"].as<glm::vec3>();
			camera.Right() = camera_node["Right"].as<glm::vec3>();
			camera.Up() = camera_node["Up"].as<glm::vec3>();
			camera.WorldUp() = camera_node["WorldUp"].as<glm::vec3>();
			camera.Yaw() = camera_node["Yaw"].as<float>();
			camera.Pitch() = camera_node["Pitch"].as<float>();
			camera.SetActive(active);
		}

		auto dir_light_node = entity["DirectionalLight"];
		if (dir_light_node)
		{
			auto &dir_light = new_entity.AddComponent<DirectionalLight>();
			dir_light.Ambient = dir_light_node["Ambient"].as<glm::vec4>();
			dir_light.Diffuse = dir_light_node["Diffuse"].as<glm::vec4>();
			dir_light.Specular = dir_light_node["Specular"].as<glm::vec4>();
		}

		auto point_light_node = entity["PointlLight"];
		if (point_light_node)
		{
			auto &point_light = new_entity.AddComponent<PointLight>();
			point_light.intensity = point_light_node["Intensity"].as<float>();
		}

		auto script_node = entity["Script"];
		if (script_node)
		{
			auto &script = new_entity.AddComponent<Script>();
			script.Bind(script_node["Binding"].as<std::string>(), new_entity);
			script.enabled = false;
		}

		auto rb2D_node = entity["RigidBody2D"];
		if (rb2D_node)
		{
			auto &rb2D = new_entity.AddComponent<RigidBody2D>();
			rb2D.type = (BodyType)rb2D_node["Type"].as<int>();
			rb2D.use_gravity = rb2D_node["UseGravity"].as<bool>();
		}

		auto mesh_collider_node = entity["MeshCollider"];
		if (mesh_collider_node)
		{
			auto &mesh_collider = new_entity.AddComponent<MeshCollider>();
			mesh_collider.type = (ColliderType)mesh_collider_node["Type"].as<int>();

			// Move to on mesh collider create callback
			switch(mesh_collider.type)
			{
				case(ColliderType::SphereCollider):
				{
					mesh_collider.collider = CreateRef<SphereCollider>();
				}break;
				case(ColliderType::PlaneCollider):
				{
					mesh_collider.collider = CreateRef<PlaneCollider>();
				}break;
				default:
					break;
			}
		}

		auto particles_node = entity["ParticleSystem"];
		if (particles_node)
		{
			auto &particles = new_entity.AddComponent<ParticleSystem>();
			particles.max_particles = particles_node["MaxParticles"].as<uint32_t>();

			particles.particle_size = particles_node["ParticleSize"].as<float>();
			particles.emission_shape = static_cast<EmissionShape>(particles_node["EmissionShape"].as<int>());
			particles.spread_angle = particles_node["SpreadAngle"].as<float>();

			particles.life_time = particles_node["LifeTime"].as<float>();
			particles.emission_rate = particles_node["EmissionRate"].as<float>();
			particles.velocity = particles_node["Velocity"].as<glm::vec2>();
			particles.running = particles_node["Running"].as<bool>();
			particles.repeating = particles_node["Repeating"].as<bool>();
			particles.world_space = particles_node["WorldSpace"].as<bool>();
			particles.duration = particles_node["Duration"].as<float>();

			particles.color_1 = particles_node["Color1"].as<glm::vec4>();
			particles.color_2 = particles_node["Color2"].as<glm::vec4>();

			auto &material = Material::Get(particles_node["Material"].as<std::string>());
			particles.material = material;
		}
	}

	return true;
}

void mrs::SceneSerializer::SerializeBinary(const std::string &out_path) {}

void mrs::SceneSerializer::SerializeEntity(YAML::Emitter &out, Entity entity)
{
	out << YAML::BeginMap;
	out << YAML::Key << "Entity" << YAML::Value << tbx::GenerateGUID();

	if (entity.HasComponent<Tag>())
	{
		out << YAML::Key << "Tag";
		out << YAML::BeginMap;

		auto &tag = entity.GetComponent<Tag>().tag;
		out << YAML::Key << "Tag" << YAML::Value << tag;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<Transform>())
	{
		out << YAML::Key << "Transform";
		out << YAML::BeginMap;

		auto &transform = entity.GetComponent<Transform>();
		out << YAML::Key << "Position" << YAML::Value << transform.position;
		out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
		out << YAML::Key << "Scale" << YAML::Value << transform.scale;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<MeshRenderer>())
	{
		out << YAML::Key << "MeshRenderer";
		out << YAML::BeginMap;

		auto &renderable = entity.GetComponent<MeshRenderer>();
		out << YAML::Key << "Mesh" << YAML::Value << renderable.GetMesh()->_mesh_name;
		out << YAML::Key << "Material" << YAML::Value << renderable.GetMaterial()->Name();

		out << YAML::EndMap;
	}

	if (entity.HasComponent<ModelRenderer>())
	{
		out << YAML::Key << "ModelRenderer";
		out << YAML::BeginMap;

		auto &renderable = entity.GetComponent<ModelRenderer>();
		out << YAML::Key << "Model" << YAML::Value << renderable.model->Name();

		out << YAML::EndMap;
	}

	if (entity.HasComponent<Script>())
	{
		out << YAML::Key << "Script";
		out << YAML::BeginMap;

		auto &script = entity.GetComponent<Script>();
		out << YAML::Key << "Binding" << YAML::Value << script.binding;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<Camera>())
	{
		out << YAML::Key << "Camera";
		out << YAML::BeginMap;

		auto &camera = entity.GetComponent<Camera>();
		out << YAML::Key << "Type" << (int)camera.GetType();
		out << YAML::Key << "Front" << YAML::Value << camera.Front();
		out << YAML::Key << "Right" << YAML::Value << camera.Right();
		out << YAML::Key << "Up" << YAML::Value << camera.Up();
		out << YAML::Key << "WorldUp" << YAML::Value << camera.WorldUp();

		out << YAML::Key << "Yaw" << YAML::Value << camera.GetYaw();
		out << YAML::Key << "Pitch" << YAML::Value << camera.GetPitch();

		out << YAML::Key << "AspectW" << YAML::Value << camera._aspect_w;
		out << YAML::Key << "AspectH" << YAML::Value << camera._aspect_h;

		out << YAML::Key << "Active" << YAML::Value << camera.IsActive();

		out << YAML::EndMap;
	}

	if (entity.HasComponent<DirectionalLight>())
	{
		out << YAML::Key << "DirectionalLight";
		out << YAML::BeginMap;

		auto &dir_light = entity.GetComponent<DirectionalLight>();
		out << YAML::Key << "Ambient" << YAML::Value << dir_light.Ambient;
		out << YAML::Key << "Diffuse" << YAML::Value << dir_light.Diffuse;
		out << YAML::Key << "Specular" << YAML::Value << dir_light.Specular;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<PointLight>())
	{
		out << YAML::Key << "PointLight";
		out << YAML::BeginMap;

		auto &point_light = entity.GetComponent<PointLight>();
		out << YAML::Key << "Intensity" << YAML::Value << point_light.intensity;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<RigidBody2D>())
	{
		out << YAML::Key << "RigidBody2D";
		out << YAML::BeginMap;

		auto &rb = entity.GetComponent<RigidBody2D>();
		out << YAML::Key << "Type" << YAML::Value << (int)rb.type;
		out << YAML::Key << "UseGravity" << YAML::Value << rb.use_gravity;

		out << YAML::EndMap;
	}

	if (entity.HasComponent<MeshCollider>())
	{
		out << YAML::Key << "MeshCollider";
		out << YAML::BeginMap;

		auto &col = entity.GetComponent<MeshCollider>();
		out << YAML::Key << "Type" << YAML::Value << (int)col.type;

		switch(col.type)
		{
			case(ColliderType::PlaneCollider):
				break;
			default:
				break;
		}

		out << YAML::EndMap;
	}

	if (entity.HasComponent<ParticleSystem>())
	{
		out << YAML::Key << "ParticleSystem";
		out << YAML::BeginMap;

		auto &particles = entity.GetComponent<ParticleSystem>();
		out << YAML::Key << "MaxParticles" << YAML::Value << particles.max_particles;
		out << YAML::Key << "ParticleSize" << YAML::Value << particles.particle_size;
		out << YAML::Key << "SpreadAngle" << YAML::Value << particles.spread_angle;
		out << YAML::Key << "EmissionShape" << YAML::Value << static_cast<int>(particles.emission_shape);
		out << YAML::Key << "LifeTime" << YAML::Value << particles.life_time;
		out << YAML::Key << "EmissionRate" << YAML::Value << particles.emission_rate;
		out << YAML::Key << "Velocity" << YAML::Value << particles.velocity;
		out << YAML::Key << "Running" << YAML::Value << particles.running;
		out << YAML::Key << "Repeating" << YAML::Value << particles.repeating;
		out << YAML::Key << "Duration" << YAML::Value << particles.duration;
		out << YAML::Key << "WorldSpace" << YAML::Value << particles.world_space;
		out << YAML::Key << "Color1" << YAML::Value << particles.color_1;
		out << YAML::Key << "Color2" << YAML::Value << particles.color_2;
		out << YAML::Key << "Material" << YAML::Value << particles.material->Name();

		out << YAML::EndMap;
	}

	out << YAML::EndMap;
}
