#include "Game/BoxHazard.hpp"
#include "Game/Player.hpp"

BoxHazard::BoxHazard(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config)
	: EnemyProjectile(gameState, entityType, startPos, config)
{
	Vec2 spriteDimensions = m_spriteBounds.GetDimensions();
	m_normalizedPivot = config.m_normalizedPivot;
}

bool BoxHazard::OverlapsPlayer(Player* player)
{
	Vec2 spriteDimensions = m_spriteBounds.GetDimensions();
	Vec2 pivotDisplacment(Lerp(-spriteDimensions.x * .5f, spriteDimensions.x * .5f, m_normalizedPivot.x),
		Lerp(-spriteDimensions.y * .5f, spriteDimensions.y * .5f, m_normalizedPivot.y));

	OBB2 hitbox;
	hitbox.m_iBasisNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
	hitbox.m_center = m_position;
	hitbox.m_center += hitbox.m_iBasisNormal * -pivotDisplacment.x;
	hitbox.m_center += hitbox.m_iBasisNormal.GetRotated90Degrees() * -pivotDisplacment.y;
	hitbox.m_halfDimensions = m_spriteBounds.GetDimensions() * .5f;

	return DoDiscAndOBBOverlap2D(player->GetPosition(), player->m_radius, hitbox);
}
