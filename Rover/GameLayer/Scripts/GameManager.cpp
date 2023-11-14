#include "GameManager.h"

#include "Planet.h"
#include "Striker.h"
#include "Moon.h"

#include "GameLayer/RenderPipelines/CBRenderPipeline.h"

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
}

void GameManager::OnStart()
{
	_scene = mrs::Application::Instance().GetScene();
	mrs::Entity cam = FindEntityWithScript<GameCamera>();

	if (cam)
	{
		_camera = cam.GetScript<GameCamera>();
		MRS_INFO("Found %s", cam.GetComponent<mrs::Tag>().tag.c_str());
	}
	else
	{
		MRS_ERROR("No game camera script found");
	}

	{
		auto planet = Instantiate("Planet 0");
		auto& transform = planet.GetComponent<mrs::Transform>();
		transform.position = glm::vec3(-10, 0, -50);
		transform.scale *= 10;

		auto& celestial_body = planet.AddComponent<CelestialBody>();
		auto& mesh_renderer = planet.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("celestial_body"));

		planet.AddComponent<mrs::Script>().Bind<Planet>();

		{
			auto moon = Instantiate("Moon 0");
			auto& moon_transform = moon.GetComponent<mrs::Transform>();
			moon_transform.position = glm::vec3(0, 0, -50.0f);
			moon_transform.scale *= 2.5;

			auto& moon_celestial_body = moon.AddComponent<CelestialBody>();
			auto& moon_mesh_renderer = moon.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("celestial_body"));

			auto& moon_props = moon.AddComponent<MoonProperties>();
			moon_props._planet = planet;
			moon_props.a = 70;
			moon_props.b = 25;
			moon.AddComponent<mrs::Script>().Bind<Moon>();
		}
	}

	// {
	// 	auto star = Instantiate("Planet 1");
	// 	auto& transform = star.GetComponent<mrs::Transform>();
	// 	transform.position = glm::vec3(10, 0, -50);
	// 	transform.scale *= 10;

	// 	auto& celestial_body = star.AddComponent<CelestialBody>();
	// 	auto& mesh_renderer = star.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("celestial_body"));

	// 	star.AddComponent<mrs::Script>().Bind<Planet>();
	// }

	{
		auto striker = Instantiate("Enemy_Striker");
		auto& transform = striker.GetComponent<mrs::Transform>();
		transform.position = glm::vec3(10, 0, 0);

		auto& mesh_renderer = striker.AddComponent<mrs::ModelRenderer>(mrs::Model::Get("striker"));

		auto& rb = striker.AddComponent<mrs::RigidBody2D>();
		striker.AddComponent<mrs::Script>().Bind<Striker>();
		rb.use_gravity = false;
	}
}

void GameManager::OnUpdate(float dt)
{
	static glm::vec2 sector_coords;

	static glm::vec2 last_sector_coords = sector_coords;

	const int sector_radius = 8;
	static std::vector<mrs::Entity> current_stars(sector_radius * sector_radius);

	const static std::string star_materials[4]
	{
		"default",
		"red",
		"green",
		"blue",
	};

	const auto& position = _camera->GetComponent<mrs::Transform>().position;

	int x = (int)(floor(position.x));
	int y = (int)(floor(position.y));

	int region_size = 64;
	sector_coords = { x - (x % region_size), y - (y % region_size) };

	// TODO: Refresh only when beyond refresh radius
	// if (sector_coords != last_sector_coords)
	// {
	// 	// Clear old star systems
	// 	for (auto& star : current_stars)
	// 	{
	// 		_scene->QueueDestroy(star);
	// 	}
	// 	current_stars.clear();

	// 	// Create new star systems
	// 	for (int i = -sector_radius / 2; i < sector_radius / 2; i++)
	// 	{
	// 		for (int j = -sector_radius / 2; j < sector_radius / 2; j++)
	// 		{
	// 			// Save current sector
	// 			last_sector_coords = sector_coords;

	// 			// Spawn Star
	// 			// glm::vec2 system_coords = { sector_coords.x + (i * region_size), sector_coords.y + (j * region_size)};

	// 			// TODO: Remove
	// 			glm::vec2 system_coords = { x, y};

	// 			// TODO: Constructor for negative coordinates as well
	// 			StarSystem system = {(uint32_t)system_coords.x, (uint32_t)system_coords.y};

	// 			if (system._star_exists)
	// 			{
	// 				MRS_INFO("NEW STAR DIAMETER = %.2f", system._star_diameter);

	// 				std::string coordinate_string = "system_" + std::to_string(static_cast<uint32_t>(system_coords.x)) + std::to_string(static_cast<uint32_t>(system_coords.y));
	// 				auto star = Instantiate(coordinate_string);

	// 				auto& transform = star.GetComponent<mrs::Transform>();
	// 				transform.position = glm::vec3(system_coords.x, system_coords.y, -50);
	// 				//transform.rotation.x = 90.0f;
	// 				transform.scale *= system._star_diameter;

	// 				auto& celestial_body = star.AddComponent<CelestialBody>();
	// 				auto& mesh_renderer = star.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("celestial_body"));

	// 				star.AddComponent<mrs::Script>().Bind<Planet>();

	// 				// Save in current sector
	// 				current_stars.push_back(star);

	// 				// TODO: Remove Premature return
	// 				return;
	// 			}
	// 		}
	// 	}
	// }
}
