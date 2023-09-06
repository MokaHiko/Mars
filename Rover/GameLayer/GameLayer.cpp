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
}

void GameLayer::OnDetatch() {};
void GameLayer::OnEnable() {};
void GameLayer::OnDisable() {};

void GameLayer::OnUpdate(float dt) {};

void GameLayer::OnImGuiRender() {};
void GameLayer::OnEvent(mrs::Event& event) {};