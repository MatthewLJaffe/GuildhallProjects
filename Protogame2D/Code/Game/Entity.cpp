#include "Game/Entity.hpp"
#include "Game/Game.hpp"

Entity::Entity(Game* game, const Vec2& startPos)
	: m_game(game)
	, m_position(startPos)
{}

Entity::~Entity()
{
	
}

void Entity::Render() const
{
	
}

void Entity::Die()
{
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
