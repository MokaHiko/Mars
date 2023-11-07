#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#pragma once

#include <ECS/ScriptableEntity.h>
#include <ECS/Components/Components.h>

#include "GameCamera.h"

class StarSystem
{
public:
	StarSystem(uint32_t x, uint32_t y, bool bGenerateFullSystem = false)
	{
		// Set seed based on location of star system
		_seed = (x & 0xFFFF) << 16 | (y & 0xFFFF);

		_star_exists = (RandInt(0, 2) == 1);
		if (!_star_exists) 
		{
			return;
		}

		_star_diameter = RandDouble(5.0, 16.0);
		_star_color = {RandDouble(0, 1), RandDouble(0, 1), RandDouble(0, 1)};
		_star_type = RandInt(0, 3);
	}

	double RandDouble(double min, double max)
	{
		return ((double)LemireRand() / (double)(0x7FFFFFFF)) * (max - min) + min;
	}

	int RandInt(int min, int max)
	{
		return (LemireRand() % (max - min)) + min;
	}

	// Modified from this for 64-bit systems:
	// https://lemire.me/blog/2019/03/19/the-fastest-conventional-random-number-generator-that-can-pass-big-crush/
	uint32_t LemireRand()
	{
		_seed += 0xe120fc15;
		uint64_t tmp;
		tmp = (uint64_t)_seed * 0x4a39b70d;
		uint32_t m1 = (tmp >> 32) ^ tmp;
		tmp = (uint64_t)m1 * 0x12fad5c9;
		uint32_t m2 = (tmp >> 32) ^ tmp;
		return m2;
	}

	void SetSeed(uint32_t seed)
	{
		_seed = seed;
	}

private:
	friend class GameManager;

	bool _star_exists = false;
	float _star_diameter = 0.0f;

	glm::vec3 _star_color = {};
	int _star_type = -1;

	uint32_t _seed = 0;
};

class GameManager : public mrs::ScriptableEntity
{
public:
    GameManager();
    ~GameManager();

    virtual void OnStart() override;
    virtual void OnUpdate(float dt) override;
private:
	mrs::Scene* _scene = nullptr;
	GameCamera* _camera = nullptr;
};

#endif