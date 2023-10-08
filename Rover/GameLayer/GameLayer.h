#ifndef GAMELAYER_H
#define GAMELAYER_H

#pragma once

#include <Mars.h>
#include "Scripts/Unit.h"
#include "Scripts/Spawner.h"
#include "Scripts/GameManager.h"
#include "Scripts/Player.h"

class GameLayer : public mrs::Layer
{
public:
    virtual void OnAttach() override;

    virtual void OnDetatch() override;
    virtual void OnEnable() override;
    virtual void OnDisable() override;

    virtual void OnUpdate(float dt) override;

    virtual void OnImGuiRender() override;
    virtual void OnEvent(mrs::Event& event) override;

private:
    void RegisterScripts();
};

#endif