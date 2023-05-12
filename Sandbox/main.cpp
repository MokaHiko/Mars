#include <iostream>

#include <Mars.h>

#include <Core/Input.h>

class SandboxApp : public mrs::Application
{
public:
	SandboxApp() {}

	~SandboxApp() {};

	virtual void LoadResources() override {
		using namespace mrs;

		//auto mesh = Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "default_mesh");
		auto mesh = Mesh::LoadFromAsset("Assets/Models/lost_empire.boop_obj", "default_mesh");

		Texture::LoadFromAsset("Assets/Models/lost_empire-RGBA.boop_png", "default_texture");
		auto mat = Material::Create("default_material");
		
		mrs::Entity e = GetScene()->Instantiate();
		e.AddComponent<mrs::RenderableObject>(mesh, mat);
	}

	virtual void OnStart() override {
		_main_camera->GetPosition() = glm::vec3(0.0, 0.0, -20.0f);
	}

	virtual void OnUpdate() override {
		using namespace mrs;
		if (Input::IsKeyPressed(SDLK_w)) {
			_main_camera->GetPosition() += glm::vec3(0.0, 0.0, 5) * GetDeltaTime();
		}
		if (Input::IsKeyPressed(SDLK_s)) {
			_main_camera->GetPosition() -= glm::vec3(0.0, 0.0, 5) * GetDeltaTime();
		}
		if (Input::IsKeyPressed(SDLK_d)) {
			_main_camera->GetPosition() += glm::vec3(5, 0.0, 0) * GetDeltaTime();
		}
		if (Input::IsKeyPressed(SDLK_a)) {
			_main_camera->GetPosition() -= glm::vec3(5, 0.0, 0) * GetDeltaTime();
		}
	}
};

mrs::Application* mrs::CreateApplication()
{
	return new SandboxApp();
}

