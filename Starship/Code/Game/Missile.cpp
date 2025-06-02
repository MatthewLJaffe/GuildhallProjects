#include "Missile.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

constexpr int NUM_TRAIL_SEGMENTS = 20;
constexpr int NUM_TRAIL_TRIS = NUM_TRAIL_SEGMENTS * 2;
constexpr int NUM_TRAIL_VERTS = NUM_TRAIL_TRIS * 3;
Rgba8 trailColor = Rgba8(150, 150, 150, 155);

Missile::Missile(Game* game, Vec2 startPos, Entity* target, float travelTime)
	: Entity(game, startPos)
{
	m_cosmeticRadius = MISSILE_COSMETIC_RADIUS;
	m_physicsRadius = MISSILE_PHYSICS_RADIUS;
	m_health = 1;
	m_typeID = MISSILE_ID;

	m_arriveTime = travelTime;	
	m_startPos = startPos;
	m_target = target;
 	m_numVerts = NUM_MISSILE_VERTS + NUM_TRAIL_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();

	Vec2 displacmentToTarget = m_target->m_position - m_startPos;
	float targetTheta = displacmentToTarget.GetOrientationDegrees();
	if (GetShortestAngularDispDegrees(m_game->m_playerShip->m_orientationDegrees, targetTheta) > 0.f)
	{
		m_controlPointDir = 1.f;
	}
	else
	{
		m_controlPointDir = -1.f;
	}

	m_target->m_targetingMissile = this;
}

void Missile::Update(float deltaSeconds)
{
	if (m_target == nullptr)
	{
		Die();
		return;
	}
	float curveHeight = .4f;
	float controlPointDistance = .2f;
	m_currTime += deltaSeconds;
	float normalizedTime = m_currTime / m_arriveTime;
	Vec2 controlPoint = Vec2::Lerp(m_startPos, m_target->m_position, controlPointDistance);
	Vec2 displacmentToTarget = m_target->m_position - m_startPos;
	if (m_controlPointDir > 0.f)
	{
		controlPoint += displacmentToTarget.GetRotated90Degrees() * curveHeight;
	}
	else
	{
		controlPoint += displacmentToTarget.GetRotatedMinus90Degrees() * curveHeight;
	}
	Vec2 startToControlLerp = Vec2::Lerp(m_startPos, controlPoint, normalizedTime);
	Vec2 controlToTargetLerp  = Vec2::Lerp(controlPoint, m_target->m_position, normalizedTime);
	Vec2 nextPos = Vec2::Lerp(startToControlLerp, controlToTargetLerp, normalizedTime);

	m_velocity = (nextPos - m_position);
	m_position = nextPos;
	m_orientationDegrees = m_velocity.GetOrientationDegrees();
	UpdateTrailSegmentPosition(normalizedTime);
	UpdateTrailOpacity(deltaSeconds);
	if (m_currTime >= m_arriveTime)
		Die();
}

void Missile::Render() const
{
	if (!IsAlive())
	{
		return;
	}
	for (int i = 0; i < m_numVerts; i++)
	{
		m_tempWorldVerts[i] = m_localVerts[i];
	}
	TransformVertexArrayXY3D(NUM_MISSILE_VERTS, m_tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	
	g_theRenderer->DrawVertexArray(NUM_MISSILE_VERTS + m_currTrailSegments * 6, m_tempWorldVerts);
}

void Missile::Die()
{
	if (m_target != nullptr)
	{
		m_target->m_targetingMissile = nullptr;
		m_game->DamageEntity(m_target, 3);
	}
	for (int i = 0; i < 3; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(-m_velocity), .1f, .2f, Rgba8(245, 66, 87, 255));
	}
	m_isDead = true;
	m_isGarbage = true;
}

void Missile::InitializeLocalVerts()
{
	Rgba8 tipColor = Rgba8(245, 66, 87, 255);
	Rgba8 baseColor = Rgba8(162, 173, 176, 255);
	Rgba8 tailColor = Rgba8(77, 58, 65, 255);

	m_localVerts[0] = Vertex_PCU(Vec3(1.5f, 0, 0), tipColor);
	m_localVerts[1] = Vertex_PCU(Vec3(1.f, .5f, 0), tipColor);
	m_localVerts[2] = Vertex_PCU(Vec3(1.f, -.5f, 0), tipColor);

	m_localVerts[3] = Vertex_PCU(Vec3(1.f, .5f, 0), baseColor);
	m_localVerts[4] = Vertex_PCU(Vec3(-1.f, .5f, 0), baseColor);
	m_localVerts[5] = Vertex_PCU(Vec3(-1.f, -.5f, 0), baseColor);

	m_localVerts[6] = Vertex_PCU(Vec3(1.f, -.5f, 0), baseColor);
	m_localVerts[7] = Vertex_PCU(Vec3(1.f, .5f, 0), baseColor);
	m_localVerts[8] = Vertex_PCU(Vec3(-1.f, -.5f, 0), baseColor);

	m_localVerts[9] = Vertex_PCU(Vec3(-1.f, .5f, 0), tailColor);
	m_localVerts[10] = Vertex_PCU(Vec3(-2.f, .75f, 0), tailColor);
	m_localVerts[11] = Vertex_PCU(Vec3(-1.f, 0.f, 0), tailColor);

	m_localVerts[12] = Vertex_PCU(Vec3(-1.f, -.5f, 0), tailColor);
	m_localVerts[13] = Vertex_PCU(Vec3(-1.f, 0.f, 0), tailColor);
	m_localVerts[14] = Vertex_PCU(Vec3(-2.f, -.75f, 0), tailColor);

	for (int i = NUM_MISSILE_VERTS; i < m_numVerts; i++)
	{
		m_localVerts[i].m_color = trailColor;
	}
}

void Missile::UpdateTrailSegmentPosition(float normalizedTime)
{
	float timePerSegment = 1.f / static_cast<float>(NUM_TRAIL_SEGMENTS);

	Vec3 missileBottomLeft(-1.f, .25f, 0.f);
	Vec3 missileBottomRight(-1.f, -.25f, 0.f);
	TransformPositionXY3D(missileBottomLeft, 1.f, m_orientationDegrees, m_position);
	TransformPositionXY3D(missileBottomRight, 1.f, m_orientationDegrees, m_position);

	int segmentStartIdx = (m_currTrailSegments - 1) * 6 + NUM_MISSILE_VERTS;
	int trailBottomLeftIdx = 2 + segmentStartIdx;
	int trailBottomRightIdx1 = segmentStartIdx;
	int trailBottomRightIdx2 =  3 + segmentStartIdx;

	int trailTopLeftIdx1 = 1 + segmentStartIdx;
	int trailTopLeftIdx2 = 5 + segmentStartIdx;
	int trailTopRightIdx = 4 + segmentStartIdx;

	//initialize bottom verts of first trail segment
	if (!m_trailInitialized)
	{
		m_localVerts[trailBottomLeftIdx].m_position = missileBottomLeft;
		m_localVerts[trailBottomRightIdx1].m_position = missileBottomRight;
		m_localVerts[trailBottomRightIdx2].m_position = missileBottomRight;
		m_trailInitialized = true;
	}

	//end of trail segment
	if (normalizedTime > timePerSegment * m_currTrailSegments && m_currTrailSegments < NUM_TRAIL_SEGMENTS)
	{
		//set top of segment in place
		m_localVerts[trailTopLeftIdx1].m_position = missileBottomLeft;
		m_localVerts[trailTopLeftIdx2].m_position = missileBottomLeft;
		m_localVerts[trailTopRightIdx].m_position = missileBottomRight;

		//set new bottom idxs equal to top
		trailBottomLeftIdx += 6;
		trailBottomRightIdx1 += 6;
		trailBottomRightIdx2 += 6;
		m_localVerts[trailBottomLeftIdx].m_position = missileBottomLeft;
		m_localVerts[trailBottomRightIdx1].m_position = missileBottomRight;
		m_localVerts[trailBottomRightIdx2].m_position = missileBottomRight;

		//get new top idxs
		trailTopLeftIdx1 += 6;
		trailTopLeftIdx2 += 6;
		trailTopRightIdx += 6;
	
		m_currTrailSegments++;
	}

	//update new top idxs
	m_localVerts[trailTopLeftIdx1].m_position = missileBottomLeft;
	m_localVerts[trailTopLeftIdx2].m_position = missileBottomLeft;
	m_localVerts[trailTopRightIdx].m_position = missileBottomRight;
}

void Missile::UpdateTrailOpacity(float deltaSeconds)
{
	m_currentOpacityDecrease += deltaSeconds * m_trailOpacityDecreaseRate;
	if (m_currentOpacityDecrease < 1.f)
	{
		return;
	}
	int alphaReduction = (int)m_currentOpacityDecrease;
	m_currentOpacityDecrease -= (float)alphaReduction;
	for (int i = NUM_MISSILE_VERTS; i < 6 * m_currTrailSegments + NUM_MISSILE_VERTS; i++)
	{
		if (m_localVerts[i].m_color.a > 0)
		{
			m_localVerts[i].m_color.a -= alphaReduction;
		}

	}
}
