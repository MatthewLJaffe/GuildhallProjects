#include "Game/Asteroid.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

Asteroid::Asteroid(Game* owner, const Vec2& pos, RandomNumberGenerator* randGen)
	: Entity(owner, pos)
{
	m_randomNumberGenerator = randGen;
	m_health = 5;
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_orientationDegrees = m_randomNumberGenerator->RollRandomFloatInRange(0.f, 360.f);
	m_velocity = Vec2(m_randomNumberGenerator->RollRandomFloatInRange(-1.f, 1.f), m_randomNumberGenerator->RollRandomFloatInRange(-1.f, 1.f)).GetNormalized() * ASTEROID_SPEED;
	m_angularVelocity = m_randomNumberGenerator->RollRandomFloatInRange(-ASTEROID_MAX_ROTATION_SPEED, ASTEROID_MAX_ROTATION_SPEED);
	m_typeID = ASTEROID_ID;
	m_numVerts = NUM_ASTEROID_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	InitializeLocalVerts();
}

Asteroid::~Asteroid()
{
	delete m_randomNumberGenerator;
}

void Asteroid::Update(float deltaSeconds)
{
	if (!IsAlive()) return;
	m_orientationDegrees += deltaSeconds * m_angularVelocity;
	m_position += m_velocity * deltaSeconds;
	if (IsOffScreen())
		WrapAround();
}

void Asteroid::InitializeLocalVerts()
{
	float firstR = m_randomNumberGenerator->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	float prevR = firstR;
	float currR;
	float thetaStep = 1.f / static_cast<float>(NUM_ASTEROID_SIDES) * 360.f;
	for (int i = 0; i < NUM_ASTEROID_SIDES; i++)
	{
		float thetaStart = static_cast<float>(i) * thetaStep;
		float thetaEnd = thetaStart + thetaStep;
		currR = m_randomNumberGenerator->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
		m_localVerts[i * 3] = Vertex_PCU(Vec3(0, 0, 0), Rgba8(100, 100, 100, 255));
		m_localVerts[i * 3 + 1] = Vertex_PCU(Vec3(CosDegrees(thetaStart) * prevR, SinDegrees(thetaStart) * prevR, 0), Rgba8(100, 100, 100, 255));
		//last vertex needs to be the same as first vertex
		if (i * 3 + 2 == NUM_ASTEROID_VERTS - 1)
			m_localVerts[i*3 + 2] = Vertex_PCU(Vec3(CosDegrees(thetaEnd) * firstR, SinDegrees(thetaEnd) * firstR, 0), Rgba8(100, 100, 100, 255));
		else
			m_localVerts[i*3 + 2] = Vertex_PCU(Vec3(CosDegrees(thetaEnd) * currR, SinDegrees(thetaEnd) * currR, 0), Rgba8(100, 100, 100, 255));
		prevR = currR;
	}
}

void Asteroid::Render() const
{
	if (!IsAlive()) return;
	Vertex_PCU tempWorldVerts[NUM_ASTEROID_VERTS];
	for (int i = 0; i < NUM_ASTEROID_VERTS; i++)
	{
		tempWorldVerts[i] = m_localVerts[i];
	}
	TransformVertexArrayXY3D(NUM_ASTEROID_VERTS, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_ASTEROID_VERTS, tempWorldVerts);
}

void Asteroid::Die()
{
	g_theAudio->StartSound(SOUND_ID_EXPLOSION);
	for (int i = 0; i < 20; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(m_velocity * 1.f), .5f, 1.f, Rgba8(100, 100, 100, 255));
	}
	m_isDead = false;
	m_isGarbage = true;
	m_game->m_currAsteroidsAlive--;
}

