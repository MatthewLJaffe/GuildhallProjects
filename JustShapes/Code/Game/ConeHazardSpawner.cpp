#include "Game/ConeHazardSpawner.hpp"
#include "Game/GameStateLevel.hpp"

ConeHazardSpawner::ConeHazardSpawner(GameState* gameState, EntityType entityType, Vec2 const& startPos, ProjectileConfig config, bool fireProjectiles)
	: EnemyProjectile(gameState, entityType, startPos, config)
	, m_fireProjectiles(fireProjectiles)
{
	m_coneExpandTimer = Timer(1.f, g_theApp->GetGameClock());
	m_coneExpandTimer.Start();
	m_minConeAngle = m_config.m_maxConeAngle * .33f;
	m_maxConeAngle = m_config.m_maxConeAngle;
}

void ConeHazardSpawner::Update(float deltaSeconds)
{
	if (!m_coneExpandTimer.HasPeriodElapsed())
	{
		m_coneAngle = Lerp(m_minConeAngle, m_maxConeAngle, SmoothStep5(m_coneExpandTimer.GetElapsedFraction()));
	}
	EnemyProjectile::Update(deltaSeconds);
}

void ConeHazardSpawner::Render()
{
	std::vector<Vertex_PCU> m_triangleVerts;
	Vec2 leftDirection = Vec2::MakeFromPolarDegrees(m_coneAngle*.5f);
	Vec2 rightDirection = Vec2::MakeFromPolarDegrees(m_coneAngle*-.5f);

	Vec2 leftPos = leftDirection * m_coneLength;
	Vec2 rightPos = rightDirection * m_coneLength;
	m_triangleVerts.push_back(Vertex_PCU(Vec3::ZERO, Rgba8::WHITE));
	m_triangleVerts.push_back(Vertex_PCU(rightPos.GetXYZ(), Rgba8::WHITE));;
	m_triangleVerts.push_back(Vertex_PCU(leftPos.GetXYZ(), Rgba8::WHITE));;

	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	Mat44 modelMatrix;

	modelMatrix.AppendTranslation2D(GetPosition());
	modelMatrix.AppendZRotation(m_orientationDegrees);
	modelMatrix.AppendScaleNonUniform2D(m_scale);
	g_theRenderer->SetModelConstants(modelMatrix, m_color);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_triangleVerts.size(), m_triangleVerts.data());
}

void ConeHazardSpawner::OnDestroy()
{
	if (m_fireProjectiles)
	{
		GameStateLevel* gameStateLevel = dynamic_cast<GameStateLevel*>(m_gameState);
		gameStateLevel->SpawnArcProjectiles(m_position + Vec2::MakeFromPolarDegrees(m_orientationDegrees) * 16.f, m_orientationDegrees, m_coneAngle);
	}
}
