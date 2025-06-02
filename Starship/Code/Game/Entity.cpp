#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Missile.hpp"

Entity::Entity(Game* game, const Vec2& startPos)
	: m_game(game)
	, m_position(startPos)
{}

Entity::~Entity()
{
	if (m_localVerts != nullptr)
	{
		delete[] m_localVerts;
		m_localVerts = nullptr;
	}
	if (m_tempWorldVerts != nullptr)
	{
		delete[] m_tempWorldVerts;
		m_tempWorldVerts = nullptr;
	}
}

void Entity::Render() const
{
	if (!IsAlive())
	{
		return;
	}
	for (int i = 0; i < m_numVerts; i++)
	{
		m_tempWorldVerts[i] = m_localVerts[i];
	}
	TransformVertexArrayXY3D(m_numVerts, m_tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(m_numVerts, m_tempWorldVerts);
}

void Entity::Die()
{
	if (m_targetingMissile != nullptr)
	{
		m_targetingMissile->m_target = nullptr;
	}
	m_isGarbage = true;
	m_isDead = true;
}

bool Entity::IsAlive() const
{
	return !m_isGarbage && !m_isDead;
}

Vec2 Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegrees);
}

bool Entity::IsOffScreen() const
{
	if (m_position.y - m_cosmeticRadius > WORLD_SIZE_Y || m_position.y + m_cosmeticRadius < 0) return true;
	if (m_position.x - m_cosmeticRadius > WORLD_SIZE_X || m_position.x + m_cosmeticRadius < 0) return true;
	return false;
}

void Entity::RenderDebug() const
{
	Vec2 forwardDir = GetForwardNormal() * m_cosmeticRadius;
	DebugDrawLine(m_position, m_game->m_playerShip->m_position, .1f, Rgba8(50, 50, 50, 255));
	DebugDrawLine(m_position, m_position + forwardDir, .1f, Rgba8(255, 0, 0, 255));
	DebugDrawLine(m_position, m_position + forwardDir.GetRotated90Degrees(), .1f, Rgba8(0, 255, 0, 255));
	DebugDrawRing(m_position, m_cosmeticRadius, .1f, Rgba8(255, 0, 255, 255));
	DebugDrawRing(m_position, m_physicsRadius, .1f, Rgba8(0, 255, 255, 255));
	DebugDrawLine(m_position, m_position + m_velocity, .1f, Rgba8(255, 255, 0, 255));
}

void Entity::WrapAround()
{
	//wrap x
	if (m_position.x - m_cosmeticRadius > WORLD_SIZE_X)
	{
		// .1f to ensure it is not continually wrapping
		m_position.x = -m_cosmeticRadius + .1f;
	}
	else if (m_position.x + m_cosmeticRadius < 0)
	{
		// .1f to ensure it is not continually wrapping
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius - .1f;
	}

	//wrap y
	if (m_position.y - m_cosmeticRadius > WORLD_SIZE_Y)
	{
		// .1f to ensure it is not continually wrapping
		m_position.y = -m_cosmeticRadius + .1f;
	}
	else if (m_position.y + m_cosmeticRadius < 0)
	{
		// .1f to ensure it is not continually wrapping
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius - .1f;
	}
}
