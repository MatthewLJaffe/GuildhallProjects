#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/App.hpp"
#include "Game/Asteroid.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Beetle.hpp"
#include "Game/MissileLauncher.hpp"
#include "Game/LaserBeam.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/Time.hpp"

PlayerShip::PlayerShip(Game* game, Vec2 startPos)
	: Entity(game, startPos)
{ 
	m_typeID = PLAYER_SHIP_ID;
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_numVerts = NUM_SHIP_VERTS;
	m_localVerts = new Vertex_PCU[m_numVerts];
	m_tempWorldVerts = new Vertex_PCU[m_numVerts];
	m_missileLauncher = new MissileLauncher(m_game);
	m_laserBeam = new LaserBeam(m_game);
	InitializeLocalVerts();
}

PlayerShip::~PlayerShip()
{
	delete m_missileLauncher;
}


void PlayerShip::InitializeLocalVerts()
{
	m_localShipVerts[0] = Vertex_PCU(Vec3(0, 2, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[1] = Vertex_PCU(Vec3(-2, 1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[2] = Vertex_PCU(Vec3(2, 1, 0), Rgba8(102, 153, 204, 255));

	m_localShipVerts[3] = Vertex_PCU(Vec3(-2, 1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[4] = Vertex_PCU(Vec3(-2, -1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[5] = Vertex_PCU(Vec3(0, 1, 0), Rgba8(102, 153, 204, 255));

	m_localShipVerts[6] = Vertex_PCU(Vec3(0, 1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[7] = Vertex_PCU(Vec3(0, -1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[8] = Vertex_PCU(Vec3(-2, -1, 0), Rgba8(102, 153, 204, 255));

	m_localShipVerts[9] = Vertex_PCU(Vec3(0, 1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[10] = Vertex_PCU(Vec3(0, -1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[11] = Vertex_PCU(Vec3(1, 0, 0), Rgba8(102, 153, 204, 255));

	m_localShipVerts[12] = Vertex_PCU(Vec3(-2, -1, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[13] = Vertex_PCU(Vec3(0, -2, 0), Rgba8(102, 153, 204, 255));
	m_localShipVerts[14] = Vertex_PCU(Vec3(2, -1, 0), Rgba8(102, 153, 204, 255));

	m_localTailVerts[0] = Vertex_PCU(Vec3(-2, -.5, 0), Rgba8(255, 0, 0, 255));
	m_localTailVerts[1] = Vertex_PCU(Vec3(-2, .5, 0), Rgba8(255, 0, 0, 255));
	m_localTailVerts[2] = Vertex_PCU(Vec3(-2, 0, 0), Rgba8(255, 0, 0, 0));
}

void PlayerShip::Update(float deltaSeconds)
{
	if (m_invincibleTime > 0.f)
	{
		m_invincibleTime -= deltaSeconds;
	}
	if (g_theInput->GetController(0).IsConnected())
	{
		HandlePlayerControlsController(deltaSeconds);
	}
	else
	{
		HandlePlayerControlsKeyboard(deltaSeconds);
	}
	MovePlayerShip(deltaSeconds);
}

void PlayerShip::HandlePlayerControlsController(float deltaSeconds)
{
	XboxController const& controller = g_theInput->GetController(0); //#ToDo: support multiple players?

	
	//Respawn
	if (m_isDead)
	{
		if (controller.WasButtonJustPressed(XboxController::START_BUTTON) && m_extraLives > 0)
		{
			Respawn();
		}
	}
	else
	{
		//Drive
		m_thrustFraction = controller.GetLeftStick().GetMagnitude();
		if (m_thrustFraction > 0.f)
		{
			m_orientationDegrees = controller.GetLeftStick().GetOrientationDegrees();
			m_thrustLength = PLAYER_SHIP_ACCELERATION * m_thrustFraction;
			m_velocity += GetForwardNormal() * deltaSeconds * m_thrustLength;
		}
		else
		{
			m_thrustLength = 0.f;
		}

		//Shoot bullet
		if (controller.WasButtonJustPressed(XboxController::A_BUTTON))
		{
			FireBullet();
		}

		//fire missiles
		m_missileLauncher->m_firing = controller.GetRightTrigger() > 0.f && m_missileLauncher->m_numAmmo > 0;
		if (m_missileLauncher->m_firing)
		{
			m_missileLauncher->FireMissiles(deltaSeconds);
		}

		//fire laser
		if (!m_laserBeam->m_firing)
		{
			if (controller.GetLeftTrigger() > 0.f && m_laserBeam->m_ammo > 0.f)
			{
				m_laserBeam->m_firing = true;
				m_laserPlayback = g_theAudio->StartSound(SOUND_ID_LASER, true);
			}
		}
		else
		{
			if (controller.GetLeftTrigger() < .001f || m_laserBeam->m_ammo <= 0.f)
			{
				m_laserBeam->m_firing = false;
				g_theAudio->StopSound(m_laserPlayback);
			}
		}

		if (m_laserBeam->m_firing)
		{
			m_laserBeam->Update(deltaSeconds);
		}
	}
}

void PlayerShip::HandlePlayerControlsKeyboard(float deltaSeconds)
{
	//Respawn
	if (m_isDead)
	{
		if (g_theInput->WasKeyJustPressed('N') && m_extraLives > 0)
		{
			Respawn();
		}
	}
	else
	{
		//turning
		if (g_theInput->IsKeyDown('S'))
		{
			m_angularVelocity = PLAYER_SHIP_TURN_SPEED;
		}
		else if (g_theInput->IsKeyDown('F'))
		{
			m_angularVelocity = -PLAYER_SHIP_TURN_SPEED;
		}
		else
		{
			m_angularVelocity = 0;
		}

		//fire bullet
		if (g_theInput->WasKeyJustPressed(' '))
		{
			FireBullet();
		}

		//fire missiles
		m_missileLauncher->m_firing = g_theInput->IsKeyDown('R') && m_missileLauncher->m_numAmmo > 0;
		if (m_missileLauncher->m_firing)
		{
			m_missileLauncher->FireMissiles(deltaSeconds);
		}

		//fire laser
		if (!m_laserBeam->m_firing)
		{
			if (g_theInput->IsKeyDown('W') && m_laserBeam->m_ammo > 0.f)
			{
				m_laserBeam->m_firing = true;
				m_laserPlayback = g_theAudio->StartSound(SOUND_ID_LASER, true);
			}
		}
		else
		{
			if (!g_theInput->IsKeyDown('W') || m_laserBeam->m_ammo <= 0.f)
			{
				m_laserBeam->m_firing = false;
				g_theAudio->StopSound(m_laserPlayback);
			}
		}
		if (m_laserBeam->m_firing)
		{
			m_laserBeam->Update(deltaSeconds);
		}

		//boost
		m_orientationDegrees += m_angularVelocity * deltaSeconds;
		if (g_theInput->IsKeyDown('E'))
		{
			m_thrustFraction = 1.f;
			m_thrustLength = PLAYER_SHIP_ACCELERATION;
			m_velocity += GetForwardNormal() * deltaSeconds * m_thrustLength;
		}
		else
		{
			m_thrustFraction = 0.f;
			m_thrustLength = 0.f;
		}
	}
}

void PlayerShip::MovePlayerShip(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;

	//check for rebounding
	if (m_position.x + m_cosmeticRadius > WORLD_SIZE_X)
	{
		m_position.x = WORLD_SIZE_X - m_cosmeticRadius;
		m_velocity.x *= -1.f;
	}
	if (m_position.x - m_cosmeticRadius < 0)
	{
		m_position.x = m_cosmeticRadius;
		m_velocity.x *= -1.f;
	}
	if (m_position.y + m_cosmeticRadius > WORLD_SIZE_Y)
	{
		m_position.y = WORLD_SIZE_Y - m_cosmeticRadius;
		m_velocity.y *= -1.f;
	}
	if (m_position.y - m_cosmeticRadius < 0)
	{
		m_position.y = m_cosmeticRadius;
		m_velocity.y *= -1.f;
	}
}

void PlayerShip::Render() const
{
	RenderShip();
	RenderTail();
	if (m_laserBeam->m_firing)
	{
		m_laserBeam->Render();
	}
}

void PlayerShip::RenderUI() const
{
	constexpr float WORLD_TO_SCREEN_RATIO = 8.f;

	std::vector<Vertex_PCU> textVerts;
	std::string missilesText = "Missiles: " + std::to_string(m_missileLauncher->m_numAmmo);
	AddVertsForTextTriangles2D(textVerts, missilesText, Vec2(150.f, 90.f) * WORLD_TO_SCREEN_RATIO, 3.f * WORLD_TO_SCREEN_RATIO, Rgba8(255, 150, 50, 255));
	std::string laserText = "Laser: " + std::to_string(static_cast<int>(ceilf(m_laserBeam->m_ammo)));
	AddVertsForTextTriangles2D(textVerts, laserText, Vec2(150.f, 80.f) * WORLD_TO_SCREEN_RATIO, 3.f * WORLD_TO_SCREEN_RATIO, Rgba8(255, 150, 50, 255));
	g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
	Vec2 offsetPos(20.f, 780.f);
	for (int i = 0; i < m_extraLives; i++)
	{
		Vertex_PCU tempWorldVerts[NUM_SHIP_VERTS];
		for (int v = 0; v < NUM_SHIP_VERTS; v++)
		{
			tempWorldVerts[v] = m_localShipVerts[v];
		}
		TransformVertexArrayXY3D(NUM_SHIP_VERTS, tempWorldVerts, 8.f, 90.f, offsetPos);
		g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, tempWorldVerts);
		offsetPos.x += 40.f;
	}
}

void PlayerShip::RenderShip() const
{
	if (m_isDead) return;
	Vertex_PCU tempWorldVerts[NUM_SHIP_VERTS];
	for (int i = 0; i < NUM_SHIP_VERTS; i++)
	{
		tempWorldVerts[i] = m_localShipVerts[i];
		if (m_invincibleTime > 0.f)
		{
			float currTime = static_cast<float>(GetCurrentTimeSeconds());
			float flashAmount = RangeMap(sinf(currTime * 10.f), -1.f, 1.f, 0.f, 1.f);
			float flashR = RangeMap(flashAmount, 0.f, 1.f, static_cast<float>(tempWorldVerts[i].m_color.r), 255.f);
			float flashG = RangeMap(flashAmount, 0.f, 1.f, static_cast<float>(tempWorldVerts[i].m_color.g), 255.f);
			float flashB = RangeMap(flashAmount, 0.f, 1.f, static_cast<float>(tempWorldVerts[i].m_color.b), 255.f);
			tempWorldVerts[i].m_color.r = static_cast<unsigned char>(flashR);
			tempWorldVerts[i].m_color.g = static_cast<unsigned char>(flashG);
			tempWorldVerts[i].m_color.b = static_cast<unsigned char>(flashB);
		}
	}


	TransformVertexArrayXY3D(NUM_SHIP_VERTS, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(NUM_SHIP_VERTS, tempWorldVerts);
}

void PlayerShip::RenderTail() const 
{
	if (m_isDead) return;
	Vertex_PCU tempWorldVerts[NUM_TAIL_VERTS];
	for (int i = 0; i < NUM_TAIL_VERTS; i++)
	{
		tempWorldVerts[i] = m_localTailVerts[i];
	}
	tempWorldVerts[NUM_TAIL_VERTS - 1].m_position = GetTailPos();
	tempWorldVerts[NUM_TAIL_VERTS - 1].m_color.a = static_cast<unsigned char>(255.f * g_randGen->RollRandomFloatInRange(0.f, .25f));
 	TransformVertexArrayXY3D(NUM_TAIL_VERTS, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexArray(NUM_TAIL_VERTS, tempWorldVerts);
}

void PlayerShip::Die()
{
	if (m_invincibleTime > 0.f)
	{
		return;
	}
	m_isDead = true;
	g_theAudio->StartSound(SOUND_ID_EXPLOSION);
	for (int i = 0; i < 15; i++)
	{
		m_game->SpawnDebris(Vec2(m_position), Vec2(m_velocity), .5f, 1.f, Rgba8(102, 153, 204, 255));
	}
	if (m_extraLives == 0)
		m_game->HandlePlayerLose();
	m_game->StartScreenShake(1.f, 1.f);
	if (m_laserBeam->m_firing)
	{
		m_laserBeam->m_firing = false;
		g_theAudio->StopSound(m_laserPlayback);
	}
	m_game->StartScreenShake(.5f, 1.5f);

}

void PlayerShip::Respawn()
{
	m_invincibleTime = 3.f;
	g_theAudio->StartSound(SOUND_ID_RESPAWN);
	m_extraLives--;
	m_health = 1;
	m_isDead = false;
	m_orientationDegrees = 0.f;
	m_position.x = WORLD_SIZE_X / 2.f;
	m_position.y = WORLD_SIZE_Y / 2.f;
	m_angularVelocity = 0.f;
	m_velocity = Vec2(0, 0);
}

void PlayerShip::RenderDebug() const
{
	Vec2 forwardDir = GetForwardNormal() * PLAYER_SHIP_COSMETIC_RADIUS;
	DebugDrawLine(m_position, m_position + m_velocity, .1f, Rgba8(255, 255, 0, 255));
	DebugDrawLine(m_position, m_position + forwardDir, .1f, Rgba8(255, 0, 0, 255));
	DebugDrawLine(m_position, m_position + forwardDir.GetRotated90Degrees(), .1f, Rgba8(0, 255, 0, 255));
	DebugDrawRing(m_position, PLAYER_SHIP_PHYSICS_RADIUS, .1f, Rgba8(0, 255, 255, 255));
	DebugDrawRing(m_position, PLAYER_SHIP_COSMETIC_RADIUS, .1f, Rgba8(255, 0, 255, 255));
}

void PlayerShip::AddMissileAmmo(int ammoToAdd)
{
	m_missileLauncher->m_numAmmo += ammoToAdd;
}

void PlayerShip::AddLaserAmmo(float ammoToAdd)
{
	m_laserBeam->m_ammo += ammoToAdd;
}

Vec3 PlayerShip::GetTailPos() const
{
	Vec3 tailPos = m_localTailVerts[NUM_TAIL_VERTS - 1].m_position;
	float minThrustFraction = .5f;
	float randT = g_randGen->RollRandomFloatInRange(m_thrustFraction * minThrustFraction, m_thrustFraction);
	float maxTailAddition = 5.f;
	tailPos.x -= Lerp(0.f, maxTailAddition, randT);
	return tailPos;
}

void PlayerShip::FireBullet()
{
	m_game->CreateBullet(m_position + GetForwardNormal() * 1.f, m_orientationDegrees);
	g_theAudio->StartSound(SOUND_ID_BULLET_SHOOT);
}
