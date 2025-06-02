#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

Player::Player(const Vec2& startPos, float startOrientation)
	:Entity(startPos, startOrientation)
{
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_turretOrientation = m_orientationDegrees;
	m_goalOrientation = m_orientationDegrees;
	m_turretTargetOrientation = m_orientationDegrees;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("tankPhysicsRadius", .25f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("tankCosmeticRadius", .75f);
	m_maxMoveSpeed = g_gameConfigBlackboard.GetValue("playerMaxMoveSpeed", 200.f);
	m_hasHealthBar = true;
	m_maxHealth = g_gameConfigBlackboard.GetValue("playerMaxHealth", 20.f);
	m_health = m_maxHealth;
	InitializeLocalVerts();
	m_entityType = ENTITY_TYPE_GOOD_PLAYER;
	m_entityFaction = EntityFaction::FACTION_GOOD;
	m_tankTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
}


Player::~Player()
{
}

void Player::Update(float deltaSeconds)
{
	m_velocity = Vec2(0.f, 0.f);
	if (g_theInput->GetController(0).IsConnected())
	{
		HandleControlsController(deltaSeconds);
	}
	else
	{
		HandleControlsKeyboard(deltaSeconds);
	}
	m_position += m_velocity * deltaSeconds;
	UpdateMuzzleFlashes();
}

void Player::HandleControlsController(float deltaSeconds)
{
	XboxController const& controller =  g_theInput->GetController(0);
	Vec2 targetDirection = controller.GetLeftStick().GetPosition();
	Vec2 targetTurretDirection = controller.GetRightStick().GetPosition();
	if (targetDirection.GetLength() > 0.f)
	{
		m_goalOrientation = targetDirection.GetOrientationDegrees();
		float prevOrientationDegrees = m_orientationDegrees;
		float tankTurnRate = g_gameConfigBlackboard.GetValue("playerTankTurnRate", 180.f);
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_goalOrientation, tankTurnRate * deltaSeconds);
		m_turretOrientation += m_orientationDegrees - prevOrientationDegrees;
		m_turretTargetOrientation += m_orientationDegrees - prevOrientationDegrees;
		m_velocity = GetForwardNormal() * m_maxMoveSpeed;
	}
	if (targetTurretDirection.GetLength() > 0.f)
	{
		m_turretTargetOrientation = targetTurretDirection.GetOrientationDegrees();
		float turretTurnRate = g_gameConfigBlackboard.GetValue("playerTurretTurnRate", 360.f);
		m_turretOrientation = GetTurnedTowardDegrees(m_turretOrientation, m_turretTargetOrientation, turretTurnRate * deltaSeconds);
	}

	if (controller.GetRightTrigger() > 0)
	{
		HandleBulletShoot(deltaSeconds);
	}
	else if (controller.GetLeftTrigger() > 0)
	{
		HandleFlameShoot(deltaSeconds);
	}
	else
	{
		m_currShootCooldown = 0.f;
	}
}

void Player::HandleControlsKeyboard(float deltaSeconds)
{
	Vec2 targetDirection = Vec2(0.f, 0.f);
	if (g_theInput->IsKeyDown('W'))
	{
		targetDirection.y += 1;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		targetDirection.x -= 1;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		targetDirection.y -= 1;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		targetDirection.x += 1;
	}
	if (targetDirection.GetLength() > 0.f)
	{
		m_goalOrientation = targetDirection.GetOrientationDegrees();
		float prevOrientationDegrees = m_orientationDegrees;
		float tankTurnRate = g_gameConfigBlackboard.GetValue("playerTankTurnRate", 180.f);
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_goalOrientation, tankTurnRate * deltaSeconds);
		m_turretOrientation += m_orientationDegrees - prevOrientationDegrees;
		m_turretTargetOrientation += m_orientationDegrees - prevOrientationDegrees;
		m_velocity = GetForwardNormal() * m_maxMoveSpeed;
	}

	Vec2 targetTurretDirection = Vec2(0.f, 0.f);
	if (g_theInput->IsKeyDown('I'))
	{
		targetTurretDirection.y += 1;
	}
	if (g_theInput->IsKeyDown('J'))
	{
		targetTurretDirection.x -= 1;
	}
	if (g_theInput->IsKeyDown('K'))
	{
		targetTurretDirection.y -= 1;
	}
	if (g_theInput->IsKeyDown('L'))
	{
		targetTurretDirection.x += 1;
	}
	if (targetTurretDirection.GetLength() > 0.f)
	{
		m_turretTargetOrientation = targetTurretDirection.GetOrientationDegrees();
		float turretTurnRate = g_gameConfigBlackboard.GetValue("playerTurretTurnRate", 360.f);
		m_turretOrientation = GetTurnedTowardDegrees(m_turretOrientation, m_turretTargetOrientation, turretTurnRate * deltaSeconds);
	}

	if (g_theInput->IsKeyDown(KEYCODE_SPACE))
	{
		HandleBulletShoot(deltaSeconds);
	}
	else if (g_theInput->IsKeyDown(KEYCODE_L_SHIFT))
	{
		HandleFlameShoot(deltaSeconds);
	}
	else
	{
		m_currShootCooldown = 0.f;
	}
}

void Player::Render() const
{
	RenderVerts(m_localVerts, m_position, m_orientationDegrees, 1.f, m_tankTexture);
	RenderVerts(m_localTurretVerts, m_position, m_turretOrientation, 1.f, m_turretTexture);
}

void Player::RenderDebug() const
{
	std::vector<Vertex_PCU> debugVerts;
	debugVerts.reserve(64*4);

	//turret orientation
	Vec2 turretDir = Vec2::MakeFromPolarDegrees(m_turretOrientation) * m_cosmeticRadius;
	AddVertsForLine2D(debugVerts, m_position, m_position + turretDir, .075f, Rgba8(0, 0, 255, 255));

	//radiuses
	AddVertsForRing2D(debugVerts, m_position, m_physicsRadius, .05f, Rgba8(0, 255, 255, 255));
	AddVertsForRing2D(debugVerts, m_position, m_cosmeticRadius, .05f, Rgba8(255, 0, 255, 255));
	if (g_theApp->m_noDamage)
	{
		AddVertsForRing2D(debugVerts, m_position, .4f, .05f, Rgba8::WHITE);
	}

	Vec2 forwardDir =  GetForwardNormal() * m_cosmeticRadius;
	//orientation
	AddVertsForLine2D(debugVerts, m_position, m_position + forwardDir, .025f, Rgba8(255, 0, 0, 255));
	AddVertsForLine2D(debugVerts, m_position, m_position + forwardDir.GetRotated90Degrees(), .025f, Rgba8(0, 255, 0, 255));

	//velocity
	AddVertsForLine2D(debugVerts, m_position, m_position + m_velocity, .015f, Rgba8(255, 255, 0, 255));

	//target orientations
	Vec2 forwardTargetDir = Vec2::MakeFromPolarDegrees(m_goalOrientation);
	AddVertsForLine2D(debugVerts, m_position + forwardTargetDir, m_position + forwardTargetDir * 1.1f, .05f, Rgba8(255, 0, 0, 255));
	Vec2 turretTargetDir = Vec2::MakeFromPolarDegrees(m_turretTargetOrientation) * m_cosmeticRadius;
	AddVertsForLine2D(debugVerts, m_position + turretTargetDir, m_position + turretTargetDir * 1.1f, .05f, Rgba8(0, 0, 255, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(debugVerts.size(), debugVerts.data());
}

void Player::Die()
{
	g_theAudio->StartSound(SOUND_ID_PLAYER_DIED);
	m_map->SpawnNewExplosion(m_position, 1.f, 1.5f);
	m_isDead = true;
	g_theGame->m_losePending = true;
}

void Player::Respawn()
{
	m_health = m_maxHealth;
	m_velocity = Vec2::ZERO;
	m_isDead = false;
}

void Player::TakeDamage(float damageAmount)
{
	if (g_theApp->m_noDamage)
	{
		return;
	}

	m_health -= damageAmount;
	if (m_health <= 0)
	{
		Die();
	}
	else
	{
		g_theAudio->StartSound(SOUND_ID_PLAYER_HIT);
	}
}

void Player::InitializeLocalVerts()
{
	AddVertsForAABB2D(m_localVerts, Vec2(-.5f, -.5f), Vec2(.5f, .5f), Rgba8(255, 255, 255, 255));
	AddVertsForAABB2D(m_localTurretVerts, Vec2(-.5f, -.5f), Vec2(.5f, .5f), Rgba8(255, 255, 255, 255));
}

void Player::UpdateMuzzleFlashes()
{
	Vec2 muzzlePos = m_position + Vec2::MakeFromPolarDegrees(m_turretOrientation) * .4f;
	for (size_t i = 0; i < m_map->m_entityListsByType[ENTITY_TYPE_MUZZLE_FLASH].size(); i++)
	{
		Entity* flash = m_map->m_entityListsByType[ENTITY_TYPE_MUZZLE_FLASH][i];
		if (m_map->IsAlive(flash))
		{
			flash->m_position = muzzlePos;
		}
	}
}

void Player::HandleBulletShoot(float deltaSeconds)
{
	m_currShootCooldown -= deltaSeconds;
	if (m_currShootCooldown <= 0.f)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Shooting a bullet");
		m_currShootCooldown = m_bulletShootCooldown;
		g_theAudio->StartSound(SOUND_ID_PLAYER_SHOOT);
		g_theGame->m_currentMap->SpawnNewEntity(ENTITY_TYPE_GOOD_BULLET, m_position + Vec2::MakeFromPolarDegrees(m_turretOrientation) * .1f, m_turretOrientation);
		m_map->SpawnNewExplosion(m_position + Vec2::MakeFromPolarDegrees(m_turretOrientation) * .4f, .3f, .2f, true);
	}
}

void Player::HandleFlameShoot(float deltaSeconds)
{
	m_currShootCooldown -= deltaSeconds;
	if (m_currShootCooldown <= 0.f)
	{
		m_currShootCooldown = m_flameShootCooldown;
		float bulletOrientation = m_turretOrientation + g_randGen->RollRandomFloatInRange(-15.f, 15.f);
		g_theGame->m_currentMap->SpawnNewEntity(ENTITY_TYPE_GOOD_FLAME_BULLET, m_position + Vec2::MakeFromPolarDegrees(m_turretOrientation) * .5f, bulletOrientation);
	}

}
