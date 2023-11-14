#ifndef STRIKER_H
#define STRIKER_H

#pragma once


#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

class Striker: public mrs::ScriptableEntity
{
public:
    void TakeDamage(float damage);
private:
    float _health = 100.0f;
};

#endif