#include "Game/Player.hpp"
#include "Game/Particle.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"

Player::Player(GameState* gameState, EntityType entityType, Vec2 const& startPos)
	: Entity(gameState, entityType, startPos)
{
	m_simulatePhysics = true;
	m_drag = 32.f;
	m_moveSpeed = 128.f;
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Player.png");
	m_spriteBounds = AABB2(Vec2::ZERO, Vec2::ONE * 8.f);
	m_radius = 4.f;
	m_spawnParticleTimer = Timer(.1f, g_theApp->GetGameClock());
	m_spawnParticleTimer.Start();
	m_dashCurve = FloatCurve::GetFloatCurveFromName("Dash");
	m_color = Rgba8(0, 229, 249);
	m_invincibilityTimer = Timer(1.5f, g_theApp->GetGameClock());
	m_dashInvincibilityTimer = Timer(m_dashInvincibilityTime, g_theApp->GetGameClock());
}

void Player::Update(float deltaSeconds)
{
	UpdateAnimation();
	HandlePlayerControls();
	UpdatePhysics(deltaSeconds);
	UpdateDash(deltaSeconds);
	UpdateInvincibility();
}

void Player::Render()
{
	Entity::Render();
}

void Player::UpdatePhysics(float deltaSeconds)
{
	if (!m_simulatePhysics)
	{
		return;
	}
	AddForce(-m_velocity * m_drag);
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_acceleration = Vec2::ZERO;
	AABB2 worldGameBounds;
	worldGameBounds.m_mins = g_theGame->m_worldCamera.GetOrthoBottomLeft();
	worldGameBounds.m_maxs = g_theGame->m_worldCamera.GetOrthoTopRight();
	Vec2 playerSize = m_spriteBounds.GetDimensions();
	if (m_position.x + playerSize.x *.5f > worldGameBounds.m_maxs.x)
	{
		m_position.x = worldGameBounds.m_maxs.x - playerSize.x * .5f;
	}
	if (m_position.y + playerSize.y * .5f > worldGameBounds.m_maxs.y)
	{
		m_position.y = worldGameBounds.m_maxs.y - playerSize.y * .5f;
	}
	if (m_position.x - playerSize.x * .5f < worldGameBounds.m_mins.x)
	{
		m_position.x = worldGameBounds.m_mins.x + playerSize.x * .5f;
	}
	if (m_position.y - playerSize.y * .5f < worldGameBounds.m_mins.y)
	{
		m_position.y = worldGameBounds.m_mins.y + playerSize.y * .5f;
	}
}

void Player::UpdateDash(float deltaSeconds)
{
	if (!m_dashInvincibilityTimer.IsStopped() && m_dashInvincibilityTimer.HasPeriodElapsed())
	{
		m_dashInvincibilityTimer.Stop();
	}
	if (!m_isDashing)
	{
		return;
	}
	m_currentDashTime += deltaSeconds;
	if (m_currentDashTime > m_totalDashTime)
	{
		m_isDashing = false;
		m_simulatePhysics = true;
		return;
	}
	float speed = m_dashCurve->EvaluateAt(m_currentDashTime / m_totalDashTime) * m_dashTopSpeed;
	if (speed < 0.f || speed > 100000.f)
	{
		return;
	}
	m_velocity = m_dashDir * speed;
	m_position += m_velocity * deltaSeconds;
}

void Player::LoseLife()
{
	if (!m_dashInvincibilityTimer.IsStopped() || m_isInvincible)
	{
		return;
	}
	g_theGame->ShakeScreen(.25f, 2.5f);
	m_lives--;
	m_isInvincible = true;
	m_invincibilityTimer.Start();
}

int Player::GetLives()
{
	return m_lives;
}

void Player::HandlePlayerControls()
{
	if (m_isDashing)
	{
		return;
	}
	XboxController const& controller = g_theInput->GetController(0);
	if (controller.IsConnected())
	{
		HandleControlsController(controller);
	}
	else
	{
		HandleControlsKeyboard();
	}
}

void Player::HandleControlsController(XboxController const& controller)
{
	Vec2 moveDir = controller.GetLeftStick().GetPosition();
	if (moveDir != Vec2::ZERO)
	{
		if (!m_moving)
		{
			m_moving = true;
			PlayAnimation("PlayerStretch");
		}
		moveDir = moveDir.GetNormalized();
		AddForce(moveDir * m_moveSpeed * m_drag);
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, moveDir.GetOrientationDegrees(), m_moveTurnSpeed * g_theApp->GetGameClock()->GetDeltaSeconds());
		if (m_spawnParticleTimer.HasPeriodElapsed())
		{
			m_spawnParticleTimer.Start();
			SpawnPlayerParticle(-moveDir);
		}

		if (controller.WasButtonJustPressed(XboxController::A_BUTTON))
		{
			if (m_dashInvincibilityTimer.IsStopped())
			{
				ParticleConfig particleConfig;
				particleConfig.m_animation = "DashRing";
				particleConfig.m_liveTime = .3f;
				particleConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Ring.png");
				particleConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(32.f, 32.f));
				m_gameState->AddEntity(new Particle(m_gameState, EntityType::PLAYER_VFX, m_position, particleConfig));

				PlayAnimation("PlayerDash");
				m_simulatePhysics = false;
				m_isDashing = true;
				m_dashInvincibilityTimer.Start();
				m_currentDashTime = 0.f;
				m_dashDir = moveDir;
				m_orientationDegrees = m_dashDir.GetOrientationDegrees();
				for (int i = 0; i < 10; i++)
				{
					SpawnPlayerParticle(-moveDir);
				}
			}
		}
	}
	else
	{
		if (m_moving)
		{
			m_moving = false;
			PlayAnimation("PlayerSquish");
		}
	}
}

void Player::HandleControlsKeyboard()
{
	Vec2 moveDir = Vec2::ZERO;
	if (g_theInput->IsKeyDown('W'))
	{
		moveDir += Vec2::UP;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveDir += Vec2::LEFT;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveDir += Vec2::DOWN;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveDir += Vec2::RIGHT;
	}
	if (moveDir != Vec2::ZERO)
	{
		if (!m_moving)
		{
			m_moving = true;
			PlayAnimation("PlayerStretch");
		}
		moveDir = moveDir.GetNormalized();
		AddForce(moveDir * m_moveSpeed * m_drag);
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, moveDir.GetOrientationDegrees(), m_moveTurnSpeed * g_theApp->GetGameClock()->GetDeltaSeconds());
		if (m_spawnParticleTimer.HasPeriodElapsed())
		{
			m_spawnParticleTimer.Start();
			SpawnPlayerParticle(-moveDir);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
		{
			if (m_dashInvincibilityTimer.IsStopped())
			{
				ParticleConfig particleConfig;
				particleConfig.m_animation = "DashRing";
				particleConfig.m_liveTime = .3f;
				particleConfig.m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Ring.png");
				particleConfig.m_spriteBounds = AABB2(Vec2::ZERO, Vec2(32.f, 32.f));
				m_gameState->AddEntity(new Particle(m_gameState, EntityType::PLAYER_VFX, m_position, particleConfig));

				PlayAnimation("PlayerDash");
				m_simulatePhysics = false;
				m_isDashing = true;
				m_dashInvincibilityTimer.Start();
				m_currentDashTime = 0.f;
				m_dashDir = moveDir;
				m_orientationDegrees = m_dashDir.GetOrientationDegrees();
				for (int i = 0; i < 10; i++)
				{
					SpawnPlayerParticle(-moveDir);
				}
			}
		}
	}
	else
	{
		if (m_moving)
		{
			m_moving = false;
			PlayAnimation("PlayerSquish");
		}
	}
}

void Player::SpawnPlayerParticle(Vec2 const& dir)
{
	Vec2 particleVelocity = dir;
	particleVelocity += Vec2::MakeFromPolarDegrees(particleVelocity.GetOrientationDegrees() +
		g_randGen->RollRandomFloatInRange(-60.f, 60.f));
	particleVelocity = particleVelocity.GetNormalized() * g_randGen->RollRandomFloatInRange(m_moveSpeed*.25f, m_moveSpeed*1.25f);
	ParticleConfig config;
	config.m_animation = "PlayerParticle";
	config.m_liveTime = .5f;
	config.m_spriteBounds = AABB2(Vec2::ZERO, Vec2::ONE * 4.f);
	config.m_texture = nullptr;
	config.m_velocity = particleVelocity;
	Particle* particle = new Particle(m_gameState, EntityType::PLAYER_VFX, m_position + dir * 4.f, config);
	particle->m_sortOrder = 6;
	m_gameState->AddEntity(particle);
}

void Player::UpdateInvincibility()
{
	if (m_isInvincible)
	{
		float newMusicVolume0To1 = RangeMapClamped(CosDegrees(m_invincibilityTimer.GetElapsedFraction() * 360.f), -1.f, 1.f, .2f, 1.f);
		g_theGame->SetMusicVolume(newMusicVolume0To1);
		if (m_invincibilityTimer.HasPeriodElapsed())
		{
			m_isInvincible = false;
			m_color.a = 255;
		}
		else
		{
			float normalizedAlpha = sinf(g_theApp->GetGameClock()->GetTotalSeconds() * 15.f);
			normalizedAlpha = RangeMap(normalizedAlpha, -1.f, 1.f, .2f, 1.f);
			m_color.a = DenormalizeByte(normalizedAlpha);
		}
	}
}
