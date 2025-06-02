#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Missile.hpp"


class MissileLauncher
{
public:
	MissileLauncher(Game* game);
	int m_numAmmo = 10;
	void FireMissiles(float deltaSeconds);
	bool m_firing = false;
private:
	float m_timeBetweenMissiles = .05f;
	float m_currTimeBetweenMissiles = 0.f;
	void ShootMissile(Entity* target);
	Game* m_game;
	int m_numPotentialTargets = 0;
};