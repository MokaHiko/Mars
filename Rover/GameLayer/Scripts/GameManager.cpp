#include "GameManager.h"
#include "Planet.h"

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
	if (sector_coords != last_sector_coords)
	{
		// Clear old star systems
		for (auto& star : current_stars)
		{
			_scene->QueueDestroy(star);
		}
		current_stars.clear();

		// Create new star systems
		for (int i = -sector_radius / 2; i < sector_radius / 2; i++)
		{
			for (int j = -sector_radius / 2; j < sector_radius / 2; j++)
			{
				// Save current sector
				last_sector_coords = sector_coords;

				// Spawn Star
				glm::vec2 system_coords = { static_cast<uint32_t>(sector_coords.x + (i * region_size)), static_cast<uint32_t>(sector_coords.y + (j) * region_size) };
				StarSystem system = {(uint32_t)system_coords.x, (uint32_t)system_coords.y};

				if (system._star_exists)
				{
					MRS_INFO("SPAWNING!");
					MRS_INFO("STAR DIAMETER = %.2f", system._star_diameter);

					std::string coordinate_string = "system_" + std::to_string(static_cast<uint32_t>(system_coords.x)) + std::to_string(static_cast<uint32_t>(system_coords.y));
					auto star = Instantiate(coordinate_string);

					auto& transform = star.GetComponent<mrs::Transform>();
					transform.position = glm::vec3(system_coords.x, system_coords.y, -100);
					transform.scale *= system._star_diameter;

					auto& mesh_renderer = star.AddComponent<mrs::MeshRenderer>(mrs::Mesh::Get("sphere"), mrs::Material::Get(star_materials[system._star_type]));

					star.AddComponent<mrs::Script>().Bind<Planet>();

					// Save in current sector
					current_stars.push_back(star);
				}
			}
		}
	}

}
