#ifndef UNIT_H
#define UNIT_H

#pragma once

#include <ECS/ScriptableEntity.h>

class Player;
class Unit : public mrs::ScriptableEntity
{
public:
    virtual void OnCreate() override;
    virtual void OnStart() override;

    virtual void OnUpdate(float dt) override;

    virtual void OnCollisionEnter2D(mrs::Entity other) override;
public:
    void MoveTo(const glm::vec2& position);
protected:
    void MoveTowards(const glm::vec2& position);
    void Die();

    float _health = 100.0f;
    float _movement_speed = 50.0f;
    glm::vec2 _target_position = {};
private:
    Player* _owner = nullptr;
};

#endif