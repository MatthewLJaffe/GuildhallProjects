#include "Game/Entity.hpp"
#include "Game/Game.hpp"

Entity::Entity(Game* game, const Vec3& startPos)
	: m_game(game)
	, m_position(startPos)
{}

Entity::Entity(Game * game, const Vec3 & startPos, EulerAngles const& orientation)
	: m_game(game)
	, m_position(startPos)
	, m_orientationDegrees(orientation)
{
}

Entity::~Entity()
{
	
}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 m_modelMatrix;
	m_modelMatrix.AppendTranslation3D(m_position);
	m_modelMatrix.Append(m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp());
	return m_modelMatrix;
}

Vec3 Entity::GetForwardNormal()
{
	return m_orientationDegrees.GetIFwd();
}

