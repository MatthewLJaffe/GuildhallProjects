#include "Game/Debris.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

Debris::Debris(Game* owner, const Vec2& pos, const Vec2& startingVelocity, float minSize, float maxSize, const Rgba8& color,  RandomNumberGenerator* randGen)
	:Entity(owner, pos)
{
	m_randomNumberGenerator = randGen;
	float size = m_randomNumberGenerator->RollRandomFloatInRange(minSize, maxSize);
	m_cosmeticRadius = size * DEBRIS_MAX_COSMETIC_RADIS;
	m_physicsRadius = size * DEBRIS_MAX_PHYSICS_RADIUS;
	m_angularVelocity = m_randomNumberGenerator->RollRandomFloatInRange(-DEBRIS_MAX_ROTATION_SPEED, DEBRIS_MAX_ROTATION_SPEED);
	float randomVelocityLength = 12.f;
	Vec2 randomVelocityComponent = Vec2(m_randomNumberGenerator->RollRandomFloatInRange(-1.f, 1.f), m_randomNumberGenerator->RollRandomFloatInRange(-1.f, 1.f)) * randomVelocityLength;
	//compute velocity by mixing random component with starting velocity
	m_velocity = startingVelocity + randomVelocityComponent;
	m_startColor = color;
	m_startColor.a = 127;
	m_orientationDegrees = m_randomNumberGenerator->RollRandomFloatInRange(0.f, 360.f);
	m_typeID = DEBRIS_ID;
	m_numVerts = NUM_DEBRIS_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

void Debris::InitializeLocalVerts()
{
	float firstR = m_randomNumberGenerator->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	float prevR = firstR;
	float currR;
	float thetaStep = 1.f / static_cast<float>(NUM_DEBRIS_SIDES) * 360.f;
	for (int i = 0; i < NUM_DEBRIS_SIDES; i++)
	{
		float thetaStart = static_cast<float>(i) * thetaStep;
		float thetaEnd = thetaStart + thetaStep;
		currR = m_randomNumberGenerator->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
		m_localVerts[i*3] = Vertex_PCU(Vec3(0, 0, 0), m_startColor);
		m_localVerts[i*3 + 1] = Vertex_PCU(Vec3(CosDegrees(thetaStart) * prevR, SinDegrees(thetaStart) * prevR, 0), m_startColor);
		//last vertex needs to be the same as first vertex
		if (i*3 + 2 == NUM_DEBRIS_VERTS - 1)
			m_localVerts[i*3 + 2] = Vertex_PCU(Vec3(CosDegrees(thetaEnd) * firstR, SinDegrees(thetaEnd) * firstR, 0), m_startColor);
		else
			m_localVerts[i*3 + 2] = Vertex_PCU(Vec3(CosDegrees(thetaEnd) * currR, SinDegrees(thetaEnd) * currR, 0), m_startColor);
		prevR = currR;
	}
}

void Debris::Update(float deltaSeconds)
{
	m_age += deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	if (m_age > DEBRIS_LIVE_TIME || IsOffScreen())
		Die();
}

void Debris::Render() const
{
	if (!IsAlive()) return;
	Vertex_PCU tempWorldVerts[NUM_DEBRIS_VERTS];
	for (int i = 0; i < NUM_DEBRIS_VERTS; i++)
	{
		tempWorldVerts[i] = m_localVerts[i];
		tempWorldVerts[i].m_color.a = static_cast<unsigned char>(Lerp(127, 0.f, m_age / DEBRIS_LIVE_TIME));
	}
	TransformVertexArrayXY3D(NUM_DEBRIS_VERTS, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_DEBRIS_VERTS, tempWorldVerts);
}

void Debris::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}
