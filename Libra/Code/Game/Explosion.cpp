#include "Game/Explosion.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

Explosion::Explosion(Vec2 const& startPos, float orientationDegrees, SpriteAnimDefinition const& animDefinition, bool isMuzzleFlash)
	: Entity(startPos, orientationDegrees)
	, m_animDefinition(animDefinition)
{
	if (isMuzzleFlash)
	{
		m_entityType = ENTITY_TYPE_MUZZLE_FLASH;
	}
	else
	{
		m_entityType = ENTITY_TYPE_EXPLOSION;
	}
	m_currTime = 0.f;
	m_duration = animDefinition.GetAnimDurationSeconds();
	m_cosmeticRadius = .5f;
	m_explosionBounds = AABB2(-.5f*m_scale*Vec2::ONE, .5f*m_scale*Vec2::ONE);
}

void Explosion::Update(float deltaSeconds)
{
	m_currTime += deltaSeconds;
	if (m_currTime >= m_duration)
	{
		Die();
	}
}

void Explosion::Render() const
{
	float currAnimTime = RangeMap(m_currTime, 0.f, m_duration, 0.f, m_animDefinition.GetAnimDurationSeconds());
	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
	const SpriteDefinition& currSpriteDef = m_animDefinition.GetSpriteDefAtTime(currAnimTime);
	std::vector<Vertex_PCU> explosionVerts;
	explosionVerts.reserve(6);
	AABB2 currSpriteUVs =  currSpriteDef.GetUVs();
	AddVertsForAABB2D(explosionVerts, m_explosionBounds, Rgba8::WHITE, currSpriteUVs.m_mins, currSpriteUVs.m_maxs);
	RenderVerts(explosionVerts, m_position, m_orientationDegrees, m_scale, currSpriteDef.GetTexture());
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
}

void Explosion::InitializeLocalVerts()
{

}
