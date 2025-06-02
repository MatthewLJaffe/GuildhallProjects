#include "Game/Scorpio.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

Scorpio::Scorpio(Vec2 const& startPos, float orientation)
	: Entity(startPos, orientation)
{
	m_isPushedByEntities = false;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_orientationDegrees = 0.f;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("tankPhysicsRadius", .25f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("tankCosmeticRadius", .75f);
	m_maxHealth = g_gameConfigBlackboard.GetValue("defaultEnemyHealth", 3.f);
	m_health = m_maxHealth;
	m_hasHealthBar = true;		
	InitializeLocalVerts();
	m_entityType = EntityType::ENTITY_TYPE_EVIL_SCORPIO;
	m_entityFaction = EntityFaction::FACTION_EVIL;
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyTurretBase.png");
	m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyCannon.png");
}

void Scorpio::Update(float deltaSeconds)
{

	Map* currMap = g_theGame->m_currentMap;
	Player* player = currMap->GetNearestPlayerAlive();
	bool trackingPlayer = false;
	if (player)
	{
		trackingPlayer = currMap->HasLineOfSight(this, m_visionRadius, player);
	}
	if (trackingPlayer)
	{
		Vec2 playerDir = player->m_position - m_position;
		m_goalOrientation = playerDir.GetOrientationDegrees();
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_goalOrientation, m_maxRotateSpeed * deltaSeconds);

		m_currentShootCooldown -= deltaSeconds;
		if ( CanShootAtPlayer() )
		{
			m_currentShootCooldown = m_shootCooldown;
			g_theAudio->StartSound(SOUND_ID_ENEMY_SHOOT);
			g_theGame->m_currentMap->SpawnNewEntity(EntityType::ENTITY_TYPE_EVIL_BULLET, m_position + GetForwardNormal() * .5f, m_orientationDegrees);
		}
	}
	else
	{
		m_orientationDegrees += m_maxRotateSpeed * deltaSeconds;
	}
	Vec2 turretPos = m_position + GetForwardNormal() * .5f;
	m_turretRaycastResult = m_map->m_opaqueMap.ImprovedRaycastVsSpecialHeat(turretPos, GetForwardNormal(), m_visionRadius, SPECIAL_TILE_HEAT);
}

void Scorpio::Render() const
{
	RenderVerts(m_localVerts, m_position, 0.f, 1.f, m_texture);
	RenderVerts(m_localTurretVerts, m_position, m_orientationDegrees, 1.f, m_turretTexture);
	
	float lineLength = m_visionRadius;
	if (m_turretRaycastResult.m_didImpact)
	{
		lineLength = m_turretRaycastResult.m_impactDist;
	}
	std::vector<Vertex_PCU> turretAimVerts;
	Vec2 forwardNormal = GetForwardNormal();
	Vec2 startPos = forwardNormal * .5f + m_position;
	AddVertsForLine2D(turretAimVerts, startPos, startPos + forwardNormal * lineLength, .05f, Rgba8(255, 0, 0, 255));
	unsigned char lineEndAlpha = static_cast<unsigned char>(Lerp(255.f, 0.f, lineLength / m_visionRadius));
	turretAimVerts[2].m_color.a = lineEndAlpha;
	turretAimVerts[3].m_color.a = lineEndAlpha;
	turretAimVerts[4].m_color.a = lineEndAlpha;
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(turretAimVerts.size(), turretAimVerts.data());
	
	
}

void Scorpio::TakeDamage(float damageAmount)
{
	m_health -= damageAmount;
	if (m_health <= 0)
	{
		Die();
	}
	else
	{
		g_theAudio->StartSound(SOUND_ID_ENEMY_HIT);
	}
}

void Scorpio::Die()
{
	m_map->SpawnNewExplosion(m_position, .75f, .75);
	g_theAudio->StartSound(SOUND_ID_ENEMY_DIED);
	Entity::Die();
	m_map->RepopulateSolidMaps();
}

void Scorpio::InitializeLocalVerts()
{
	AddVertsForAABB2D(m_localVerts, Vec2(-.5f, -.5f), Vec2(.5f, .5f), Rgba8(255, 255, 255, 255));
	AddVertsForAABB2D(m_localTurretVerts, Vec2(-.5f, -.5f), Vec2(.5f, .5f), Rgba8(255, 255, 255, 255));
}

bool Scorpio::CanShootAtPlayer()
{
	return m_currentShootCooldown <= 0.f && fabsf(m_orientationDegrees - m_goalOrientation) < 5.f;
}

