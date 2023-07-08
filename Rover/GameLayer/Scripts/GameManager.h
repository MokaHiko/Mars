#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <Toolbox/RandomToolBox.h>
#include <ECS/Components/Components.h>
#include <Core/Application.h>
#include <Physics/Physics.h>

class GameManager : public mrs::ScriptableEntity
{
public:
    void OnStart() override
    {

    }
private:
    mrs::Scene* _scene;
};

#endif