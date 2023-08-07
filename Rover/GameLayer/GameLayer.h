#ifndef GAMELAYER_H
#define GAMELAYER_H

#pragma once

#include <Mars.h>

class GameLayer : public mrs::Layer
{
public:
    virtual void OnAttach() override 
    {
        auto height_map = mrs::Texture::LoadFromAsset("Assets/Textures/iceland_heightmap.boop_png", "iceland_height_map");

        auto terrain = mrs::Application::GetInstance().GetScene()->Instantiate("IcelandTerrain");
        auto& terrainRenderer = terrain.AddComponent<mrs::TerrainRenderer>();
        terrainRenderer.height_map = height_map;
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