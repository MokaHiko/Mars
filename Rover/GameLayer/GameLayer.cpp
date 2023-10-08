#include "GameLayer.h"

void GameLayer::OnAttach()
{
	RegisterScripts();
};

void GameLayer::RegisterScripts()
{
	mrs::Script::Register<Unit>();
	mrs::Script::Register<Spawner>();
	mrs::Script::Register<GameManager>();
	mrs::Script::Register<Player>();

	// TODO: Load all assets in asset folder
	//Model::LoadFromAsset("Assets/Models/Room.bp", "room");
}

void GameLayer::OnDetatch() {};
void GameLayer::OnEnable() {};
void GameLayer::OnDisable() {};

void GameLayer::OnUpdate(float dt) {};

void GameLayer::OnImGuiRender() {};
void GameLayer::OnEvent(mrs::Event& event) {};