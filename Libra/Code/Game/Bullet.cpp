#include "Game/Bullet.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"


Bullet::Bullet(Vec2 const& startPos, float startOrientation, EntityType entityType)
	: Entity(startPos, startOrientation)
{
	m_entityType = entityType;
	if (m_entityType == ENTITY_TYPE_GOOD_BULLET)
	{
		m_entityFaction = FACTION_GOOD;
		m_health = 3;
		m_entityType = ENTITY_TYPE_GOOD_BULLET;
		m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FriendlyBolt.png");
	}
	if (m_entityType == ENTITY_TYPE_EVIL_BULLET)
	{
		m_entityFaction = FACTION_EVIL;
		m_health = 1;
		m_entityType = ENTITY_TYPE_EVIL_BULLET;
		m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/EnemyBolt.png");
	}
	if (m_entityType == ENTITY_TYPE_GOOD_FLAME_BULLET)
	{
		if (g_randGen->RollRandomFloatZeroToOne() > .5f)
		{
			m_rotationSpeed = g_randGen->RollRandomFloatInRange(-500.f, -100.f);
		}
		else
		{
			m_rotationSpeed = g_randGen->RollRandomFloatInRange(100.f, 500.f);
		}
		m_maxSpeed = 3.f;
		m_maxLifetime = .75f;
		m_health = 1;
		m_animDefinition = ANIM_DEFINITION_EXPLOSION;
		m_cosmeticRadius = .5f;
		m_physicsRadius = .05f;
	}
	else
	{
		m_cosmeticRadius = .25f;
		m_maxLifetime = 3.f;
		m_physicsRadius = .1f;
	}
	m_velocity = GetForwardNormal() * m_maxSpeed;
	InitializeLocalVerts();
}

void Bullet::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_rotationSpeed * deltaSeconds;
	Vec2 reboundNormal = CheckForRebound();
	if (reboundNormal != Vec2::ZERO)
	{
		ReflectBullet(reboundNormal);
		Tile& collisionTile = g_theGame->m_currentMap->GetTileFromPos(m_position - reboundNormal.GetNewLength(m_physicsRadius));
		PushDiscOutOfFixedAABB2D(m_position, m_physicsRadius, collisionTile.GetTileBounds());

		
		//handle destructible tiles
		if (m_entityFaction == FACTION_GOOD && collisionTile.m_tileDef->m_isDestructible)
		{
			collisionTile.m_health -= m_bulletDamage;
			if (collisionTile.m_health <= 0.f)
			{
				collisionTile.m_tileDef = TileDefinition::GetTileDefFromName(collisionTile.m_tileDef->m_destroyedTileType);
				collisionTile.m_health = collisionTile.m_tileDef->m_maxHealth;
				m_map->RepopulateSolidMaps();
				m_map->UpdateMapVerts();
			}
		}
		
	}
	m_currLiveTime += deltaSeconds;
	if (m_currLiveTime >= m_maxLifetime)
	{
		Die();
	}
}

void Bullet::InitializeLocalVerts()
{
	AddVertsForAABB2D(m_localVerts, Vec2(-.075f, -.03f), Vec2(.075f, .03f), Rgba8::WHITE);
}

Vec2 Bullet::CheckForRebound()
{
	IntVec2 tileCoords = g_theGame->m_currentMap->GetCoordsFromPos(m_position);

	if (IsInReboundingTile(m_position + Vec2::UP * m_physicsRadius))
		return Vec2::DOWN;
	if (IsInReboundingTile(m_position + Vec2::DOWN * m_physicsRadius))
		return Vec2::UP;
	if (IsInReboundingTile(m_position + Vec2::LEFT * m_physicsRadius))
		return Vec2::RIGHT;
	if (IsInReboundingTile(m_position + Vec2::RIGHT * m_physicsRadius))
		return Vec2::LEFT;

	if (IsInReboundingTile(m_position + Vec2::TOP_RIGHT * m_physicsRadius))
		return Vec2::BOTTOM_LEFT;
	if (IsInReboundingTile(m_position + Vec2::BOTTOM_RIGHT * m_physicsRadius))
		return Vec2::TOP_LEFT;
	if (IsInReboundingTile(m_position + Vec2::BOTTOM_LEFT * m_physicsRadius))
		return Vec2::TOP_RIGHT;
	if (IsInReboundingTile(m_position + Vec2::TOP_LEFT * m_physicsRadius))
		return Vec2::BOTTOM_RIGHT;

	return Vec2::ZERO;
}

void Bullet::ReflectBullet(Vec2 const& surfaceNormal, bool ariesShield)
{
	if (ariesShield)
	{
		g_theAudio->StartSound(SOUND_ID_BULLET_RICOCHET2);
	}
	else
	{
		g_theAudio->StartSound(SOUND_ID_BULLET_RICOCHET1);
	}
	m_velocity.Reflect(surfaceNormal);
	Vec2 normalPerp = surfaceNormal.GetRotated90Degrees();
	m_velocity += normalPerp * g_randGen->RollRandomFloatInRange(-m_bounceRandomness, m_bounceRandomness);
	m_velocity.SetLength(m_maxSpeed);
	TakeDamage(1.f);
	m_orientationDegrees = m_velocity.GetOrientationDegrees();
}

bool Bullet::IsInReboundingTile(Vec2 const& point)
{
	Map& currentMap = *(g_theGame->m_currentMap);
	return currentMap.IsPointInSolid(point, false);
}



void Bullet::Render() const
{
	if (m_entityType == ENTITY_TYPE_GOOD_FLAME_BULLET)
	{
		float currAnimTime = RangeMap(m_currLiveTime, 0.f, m_maxLifetime, 0.f, m_animDefinition->GetAnimDurationSeconds());
		SpriteDefinition const& spriteDef =  m_animDefinition->GetSpriteDefAtTime(currAnimTime);
		std::vector<Vertex_PCU> localVerts;
		AABB2 const& spriteUVs = spriteDef.GetUVs();
		AddVertsForAABB2D(localVerts, AABB2(Vec2(-.5f, -.5f), Vec2(.5f, .5f)), Rgba8::WHITE, spriteUVs.m_mins, spriteUVs.m_maxs);
		g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
		RenderVerts(localVerts, m_position, m_orientationDegrees, 1.f, spriteDef.GetTexture());
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	}
	else
	{
		RenderVerts(m_localVerts, m_position, m_orientationDegrees, 1.f, m_texture);
	}
}

void Bullet::Die()
{
	if (m_entityType != ENTITY_TYPE_GOOD_FLAME_BULLET)
	{
		m_map->SpawnNewExplosion(m_position, .5f, .5f);
	}
	m_isDead = true;
	m_isGarbage = true;
}

