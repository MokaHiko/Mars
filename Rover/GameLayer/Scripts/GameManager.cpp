#include "GameManager.h"

#include <Toolbox/RandomToolBox.h>
#include <UI/UI.h>

#include "Planet.h"
#include "Ship.h"
#include "PlayerShipController.h"
#include "Striker.h"
#include "Moon.h"

#include "GameLayer/RenderPipelines/CBRenderPipeline.h"

#include "Effects/TargetLockEffect.h"

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
		// auto& sky = Instantiate("SkyBox");
		// sky.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("sphere"), mrs::Material::Get("stars"));
		// sky.GetComponent<mrs::Transform>().position.z = 400.0f;
		// sky.GetComponent<mrs::Transform>().scale *= 1500.0f;
	}

	tbx::PRNGenerator<float> random(0, 1);
	tbx::PRNGenerator<float> random_neg(-0.5f, 0.5f);

	int root = 1;
	float spacing = 100;
	for (int i = 0; i < root; i++)
	{
		for (int j = 0; j < root; j++)
		{
			auto planet = Instantiate(std::string("Planet") + std::to_string(i + j));
			auto& transform = planet.GetComponent<mrs::Transform>();
			transform.position = glm::vec3(-spacing * i, spacing * j, -950);
			transform.scale *= 500 * (1 + (0.05f) * random_neg.Next());

			auto& celestial_body = planet.AddComponent<CelestialBody>();
			NoiseFilter noise_filter = {};
			noise_filter.noise_settings.n_layers = 4;
			noise_filter.noise_settings.persistence = 0.5;
			noise_filter.noise_settings.base_roughness = 1.2f;
			noise_filter.noise_settings.roughness = 2.0f;
			noise_filter.noise_settings.strength = 0.250;
			noise_filter.noise_settings.min_value = 0.928;
			noise_filter.noise_settings.min_resolution = 32;
			noise_filter.noise_settings.max_resolution = 128;
			noise_filter.noise_settings.mask = 1;
			celestial_body.PushFilter(noise_filter);

			noise_filter.noise_settings.n_layers = 2;
			noise_filter.noise_settings.persistence = 0.630;
			noise_filter.noise_settings.base_roughness = 0.200f;
			noise_filter.noise_settings.roughness = 5.700;
			noise_filter.noise_settings.strength = 1.880;
			noise_filter.noise_settings.min_value = 1.178;
			celestial_body.PushFilter({noise_filter});

			auto& mesh_renderer = planet.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("celestial_body"));

			auto& planet_props = planet.AddComponent<PlanetProperties>();
			planet_props.axis_of_rotation = mrs::Vector3(0.5, 0.25, 0.8);
			planet_props.rotation_rate = 5.0f;

			planet.AddScript<Planet>();
			{
				auto moon = Instantiate("Moon 0");
				auto& moon_transform = moon.GetComponent<mrs::Transform>();
				moon_transform.position = planet.GetComponent<mrs::Transform>().position;
				moon_transform.scale = planet.GetComponent<mrs::Transform>().scale * 0.25f;

				auto& moon_celestial_body = moon.AddComponent<CelestialBody>();
				NoiseFilter noise_filter = {};
				noise_filter.noise_settings.n_layers = 4;
				noise_filter.noise_settings.persistence = 0.920f * random.Next();
				noise_filter.noise_settings.base_roughness = 1.2f * (1 + random.Next());
				noise_filter.noise_settings.roughness = 2.0f * (1 + random_neg.Next());
				noise_filter.noise_settings.min_value = 1.0f * random.Next();
				noise_filter.noise_settings.strength = 0.3f * (1 + random.Next());
				noise_filter.noise_settings.min_value = 1.880 * random.Next();
				moon_celestial_body.PushFilter(noise_filter);
				auto& moon_mesh_renderer = moon.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("cube"), mrs::Material::Get("celestial_body"));

				auto& moon_props = moon.AddComponent<MoonProperties>();
				moon_props._planet = planet;
				moon_props.a = 7 * planet.GetComponent<mrs::Transform>().scale.x;
				moon_props.b = 2.5f * planet.GetComponent<mrs::Transform>().scale.x;
				moon.AddScript<Moon>();
			}
		}
	}

	{
		// Player
		auto player = Instantiate("Player");
		auto& transform = player.GetComponent<mrs::Transform>();
		transform.position = glm::vec3(20, 20, 0);

		// Ship
		auto& specs = player.AddComponent<ShipSpecs>();
		specs.model = mrs::Model::Get("zenith");
		specs.ms = 5.0f;
		specs.max_speed = 35.0f;
		player.AddScript<Ship>();

		// Ship controller
		auto ship_controller = Instantiate("Player Controller!");
		ship_controller.AddScript<PlayerShipController>();

		// Parent
		transform.AddChild(ship_controller);
	}

	for (int i = 1; i < 5; i++) 
	{
		// Enemy
		auto enemy = Instantiate("Enemy_Striker");
		auto& transform = enemy.GetComponent<mrs::Transform>();
		transform.position = glm::vec3(10 * i, 50.0f, 0);
		transform.scale *= 2.0f;

		// Ship
		auto& specs = enemy.AddComponent<ShipSpecs>();
		specs.model = mrs::Model::Get("striker");
		enemy.AddScript<Ship>();

		// // Enemy controller
		auto enemy_controller = Instantiate("Enemy Controller!");
		enemy_controller.AddScript<Striker>();
		transform.AddChild(enemy_controller);

		// TODO: Move to targetting
		auto targetting = Instantiate("targetting");
		targetting.AddComponent<EffectProperties>().duration = 10.0f;
		targetting.AddComponent<EffectProperties>().fixed_time = false;
		targetting.AddScript<TargetLockEffect>();
		transform.AddChild(targetting);
	}

	// Ui 
	{
		// WorldSpace 
	}
}

void GameManager::OnUpdate(float dt)
{
	// static glm::vec2 sector_coords;

	// static glm::vec2 last_sector_coords = sector_coords;

	// const int sector_radius = 8;
	// static std::vector<mrs::Entity> current_stars(sector_radius * sector_radius);

	// const static std::string star_materials[4]
	// {
	// 	"default",
	// 	"red",
	// 	"green",
	// 	"blue",
	// };

	// const auto& position = _camera->GetComponent<mrs::Transform>().position;

	// int x = (int)(floor(position.x));
	// int y = (int)(floor(position.y));

	// int region_size = 64;
	// sector_coords = { x - (x % region_size), y - (y % region_size) };

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

	// 				star.AddScript<Planet>();

	// 				// Save in current sector
	// 				current_stars.push_back(star);

	// 				// TODO: Remove Premature return
	// 				return;
	// 			}
	// 		}
	// 	}
	// }
}
