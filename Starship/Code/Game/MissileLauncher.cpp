#include "MissileLauncher.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

MissileLauncher::MissileLauncher(Game* game)
	: m_game(game)
{ }

void MissileLauncher::FireMissiles(float deltaSeconds)
{
	m_currTimeBetweenMissiles += deltaSeconds;
	if (m_currTimeBetweenMissiles >= m_timeBetweenMissiles)
	{
		float maxAlignmentWithPlayer = .5f;
		Entity* target = m_game->GetMissileTarget(maxAlignmentWithPlayer);
		if (target != nullptr)
		{
			m_numAmmo--;
			m_currTimeBetweenMissiles = 0.f;
			ShootMissile(target);
			target->m_missileLockedOn = true;
		}
	}
}

void MissileLauncher::ShootMissile(Entity* target)
{
	PlayerShip* playerShip = m_game->m_playerShip;
	Vec2 playerForwardNormal = playerShip->GetForwardNormal();
	m_game->SpawnMissile(playerShip->m_position + playerForwardNormal, target);
}
