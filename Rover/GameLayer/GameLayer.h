#ifndef GAMELAYER_H
#define GAMELAYER_H

#pragma once

#include <Mars.h>

#include "Scripts/GameManager.h"
#include "Scripts/PlayerCamera.h"
#include "Scripts/PlayerController.h"
#include "Scripts/Unit.h"

class GameLayer : public mrs::Layer
{
public:
    virtual void OnAttach() override 
    {
        // Load Game Resources
		mrs::Mesh::LoadFromAsset("Assets/Models/monkey_smooth.boop_obj", "monkey");

        // Register Scripts
        mrs::Script::Register<GameManager>();
        mrs::Script::Register<PlayerController>();
        mrs::Script::Register<PlayerCamera>();
        mrs::Script::Register<Unit>();
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