#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/HeatMaps.hpp"

class Game;
class App;
class Renderer;
class Bullet;
class GuidedMissile;
class Map;


enum EntityType
{
	ENTITY_TYPE_GOOD_BULLET,
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_GOOD_BOLT,
	ENTITY_TYPE_EVIL_BOLT,
	ENTITY_TYPE_EVIL_SCORPIO,
	ENTITY_TYPE_EVIL_LEO,
	ENTITY_TYPE_EVIL_ARIES,
	ENTITY_TYPE_GOOD_PLAYER,
	ENTITY_TYPE_EVIL_CAPRICORN,
	ENTITY_TYPE_EVIL_MISSILE,
	ENTITY_TYPE_EXPLOSION,
	ENTITY_TYPE_MUZZLE_FLASH,
	ENTITY_TYPE_GOOD_FLAME_BULLET,
	NUM_ENTITY_TYPES
};

enum EntityFaction
{
	FACTION_GOOD,
	FACTION_NEUTRAL,
	FACTION_EVIL,
	NUM_FACTION_TYPES
};


class Entity
{

public:
	Entity(Vec2 const& startPos);
	Entity(Vec2 const& startPos, float orientationDegrees);
	virtual ~Entity();
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const;
	void RenderHealthBar() const;
	virtual void RenderDebug() const;
	virtual void Die();
	virtual void HandleIncomingBullet(Bullet* bullet);
	virtual void HandleIncomingMissile(GuidedMissile* missile);
	virtual void TakeDamage(float amount);
	Vec2 GetForwardNormal() const;
	bool IsAlive() const;
	void MoveTankTowardsPoint(Vec2 const& point, float deltaSeconds);
	void ComputeWayPointPos();
	void ComputeWayPointPosImproved();
protected:
	virtual void InitializeLocalVerts() = 0;
	void RenderVerts(std::vector<Vertex_PCU> const& verts, Vec2 const& position, float orientationDegrees, float scaleXY, Texture* tex) const;
	void Wander(float deltaSeconds);
	void UpdateTargetPos();
	void MoveTowardsWaypointPos(float deltaSeconds);
	float m_visionRadius = g_gameConfigBlackboard.GetValue("enemyVisionRadius", 10.f);
	bool m_isWandering = false;
	bool m_inPursuitOfPlayer = false;
public:
	TileHeatMap* m_heatMapOfTargetPos = nullptr;
	Map* m_map = nullptr;
	Vec2 m_position;
	Vec2 m_velocity = Vec2(0, 0);
	std::vector<Vertex_PCU> m_localVerts;
	float m_orientationDegrees = 0.f;
	float m_angularVelocity = 0.f;
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;
	float m_health = 1.f;
	float m_maxHealth = 1.f;
	bool m_hasHealthBar = false;
	bool m_isDead = false;
	bool m_canSwim = false;
	bool m_isGarbage = false;
	EntityType m_entityType;
	EntityFaction m_entityFaction = EntityFaction::FACTION_NEUTRAL;
	float m_maxMoveSpeed = 1.f;
	float m_maxRotateSpeed = 180.f;
	float m_changeDirectionTime = 1.f;
	float m_currentDirectionTime = 0.f;
	float m_goalOrientation = 0.f;

	bool m_isPushedByEntities = false;
	bool m_doesPushEntities = false;
	bool m_isPushedByWalls = false;
	bool m_isHitByBullets = false;

	Vec2 m_targetPos = Vec2::ZERO;
	std::vector<Vec2> m_pathPoints;
	Vec2 m_nextWayPointPos = Vec2::ZERO;
	
};

typedef std::vector<Entity*> EntityList;
