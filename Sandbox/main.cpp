#include <iostream>

#include <Mars.h>

class SandboxApp : public mrs::Application
{
public:
	SandboxApp() {}

	~SandboxApp() {};

	virtual void LoadResources() override {
		auto my_mesh = mrs::Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj");
		//auto my_mesh = mrs::Mesh::Create("default_triangle");

		////vertex positionsk
		//my_mesh->_vertices.resize(3);
		//my_mesh->_vertices[0].position = { 1.f, 1.f, 0.0f };
		//my_mesh->_vertices[1].position = { -1.f, 1.f, 0.0f };
		//my_mesh->_vertices[2].position = { 0.f,-1.f, 0.0f };

		////vertex colors, all green
		//my_mesh->_vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
		//my_mesh->_vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
		//my_mesh->_vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green
	}
};

mrs::Application* mrs::CreateApplication()
{
	return new SandboxApp();
}

