#ifndef PLAYER_H
#define PLAYER_H

#pragma once


#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include <Scripts/Editor/CameraController.h>
class Player : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() override;
    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Entity other) override;
public:
    void Select(mrs::Entity unit);
private:
    std::vector<glm::vec2> GenerateBoxPositions(const glm::vec2& position, uint32_t n_positions);
private:
    mrs::CameraController* _camera = nullptr;
    mrs::Scene* _scene = nullptr;
    std::vector<mrs::Entity> _selected_units;
};

#endif