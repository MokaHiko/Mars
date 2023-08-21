#ifndef GAMELAYER_H
#define GAMELAYER_H

#pragma once

#include <Mars.h>
#include "Scripts/Unit.h"
#include "Scripts/Spawner.h"
#include "Scripts/GameManager.h"

class GameLayer : public mrs::Layer
{
public:
    virtual void OnAttach() override 
    {
        // Register scripts
        mrs::Script::Register<Unit>();
        mrs::Script::Register<Spawner>();
        mrs::Script::Register<GameManager>();

        mrs::Scene* scene = mrs::Application::GetInstance().GetScene();

        for(uint32_t i = 0; i < 10; i++)
        {
            mrs::Entity e = scene->Instantiate("Spawner" + std::to_string(i), glm::vec3(i * 5.0f, 5.0f, 0.0f));
            e.AddComponent<mrs::Script>().Bind<Spawner>();
        }

        for(uint32_t i = 0; i < 10 ; i++)
        {
            mrs::Entity e = scene->Instantiate("box" + std::to_string(i), glm::vec3(i * 5.0f, 5.0f, 0.0f));

            auto& renderable = e.AddComponent<mrs::RenderableObject>();
            renderable.SetMesh(mrs::Mesh::Get("cube"));

            e.AddComponent<mrs::BoxCollider2D>();
            e.AddComponent<mrs::RigidBody2D>();
            e.AddComponent<mrs::Script>().Bind<Unit>();
        }
    };

    virtual void OnDetatch() override 
    {
        
    };

    virtual void OnEnable() override
    {

    };

    virtual void OnDisable() override
    {

    };

    virtual void OnUpdate(float dt) override 
    {

    };

    virtual void OnImGuiRender() override 
    {

    };

    virtual void OnEvent(mrs::Event& event) override 
    {

    };
private:

};

#endif