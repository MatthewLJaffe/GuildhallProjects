#include "Game/Player.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

Player::Player(Game* game, int playerIndex, int controllerIndex)
	: m_game(game)
	, m_playerIndex(playerIndex)
	, m_controllerIndex(controllerIndex)
	, m_scoreReduceTimer(Timer(.2f, g_theApp->GetGameClock()))
	, m_startGradeReductionTimer(.75f, g_theApp->GetGameClock())
{
	m_worldCamera.SetPerspectiveView(g_gameConfigBlackboard.GetValue("aspectRatio", 2.f), 60.f, .1f, 100.f);
	m_worldCamera.m_mode = Camera::eMode_Perspective;
	m_worldCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2::ZERO, GetScreenDimensions());
	m_isPlayer = true;
	m_textBoxTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TextBox.png");
	m_defaultHudTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Hud_Base.png");
}

Player::~Player()
{

}

void Player::RenderScreen()
{
	if (m_freeFlyCameraMode || GetActor()->m_actorDefinition->m_name != "Marine")
	{
		return;
	}

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	
	Vec2 screenDimensions = GetScreenDimensions();
	AABB2 screenHudBounds;
	screenHudBounds.m_mins = Vec2::ZERO;
	screenHudBounds.m_maxs = Vec2(screenDimensions.x, screenDimensions.y * .147f);
	std::vector<Vertex_PCU> hudVerts;
	AddVertsForAABB2D(hudVerts, screenHudBounds, Rgba8::WHITE);
	g_theRenderer->BindTexture(m_defaultHudTexture);
	g_theRenderer->DrawVertexArray(hudVerts.size(), hudVerts.data());

	std::vector<Vertex_PCU> textVerts; 
	g_bitMapFont->AddVertsForText2D(textVerts, Vec2(.05f, .06f) * screenDimensions, 50.f, Stringf("%d", m_kills));
	g_bitMapFont->AddVertsForText2D(textVerts, Vec2(.925f, .06f) * screenDimensions, 50.f, Stringf("%d", m_deaths));
	g_bitMapFont->AddVertsForText2D(textVerts, Vec2(.2575f, .06f) * screenDimensions, 50.f, Stringf("%1.f", GetActor()->m_currentHealth));
	g_theRenderer->BindTexture(g_bitMapFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts.size(), textVerts.data());

	if (GetActor()->m_isDead)
	{
		std::vector<Vertex_PCU> overlayVerts;
		AABB2 screenOverlay(Vec2::ZERO, screenDimensions);
		AddVertsForAABB2D(overlayVerts, screenOverlay, Rgba8(0, 0, 0, 175));
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(overlayVerts.size(), overlayVerts.data());
	}
	else
	{
		if (GetActor()->GetCurrentWeapon() != nullptr)
		{
			WeaponDefinition const& weaponDef = *GetActor()->GetCurrentWeapon()->m_definition;

			std::vector<Vertex_PCU> reticleVerts;
			Vec2 screenCenter = screenDimensions * .5f;
			AddVertsForAABB2D(reticleVerts, AABB2(screenCenter - weaponDef.m_reticleSize * .5f, screenCenter + weaponDef.m_reticleSize * .5f), Rgba8::WHITE);
			g_theRenderer->BindTexture(weaponDef.m_reticleTexture);
			g_theRenderer->DrawVertexArray(reticleVerts.size(), reticleVerts.data());

			AABB2 weaponBounds;
			Vec2 spriteMinXRange(0.f, screenDimensions.x - weaponDef.m_spriteSize.x);
			Vec2 spriteMinYRange(screenHudBounds.m_maxs.y, screenDimensions.y - weaponDef.m_spriteSize.y);

			weaponBounds.m_mins.x = Lerp(spriteMinXRange.x, spriteMinXRange.y, weaponDef.m_spritePivot.x);
			weaponBounds.m_mins.y = Lerp(spriteMinYRange.x, spriteMinYRange.y, weaponDef.m_spritePivot.y);
			weaponBounds.m_maxs = weaponBounds.m_mins + weaponDef.m_spriteSize;

			std::vector<Vertex_PCU> weaponVerts;
			SpriteDefinition const& weaponSprite = GetActor()->GetCurrentWeapon()->GetCurrentAnimationFrame();
			AABB2 weaponUVs = weaponSprite.GetUVs();
			AddVertsForAABB2D(weaponVerts, weaponBounds, Rgba8::WHITE, weaponUVs.m_mins, weaponUVs.m_maxs);
			g_theRenderer->BindTexture(weaponSprite.GetTexture());
			g_theRenderer->DrawVertexArray(weaponVerts.size(), weaponVerts.data());
		}
	}

	if (m_showErrorMessage)
	{
		Vec2 screenCenter = screenDimensions * .5f;
		Vec2 errorDimensions(388, 277);
		AABB2 errorBounds(screenCenter - errorDimensions*.7f, screenCenter + errorDimensions*.7f);
		std::vector<Vertex_PCU> errorVerts;
		AddVertsForAABB2D(errorVerts, errorBounds, Rgba8::WHITE);
		g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ErrorMessage.png"));
		g_theRenderer->DrawVertexArray(errorVerts.size(), errorVerts.data());
	}
}

void Player::Update(float deltaSeconds)
{
	if (m_freeFlyCameraMode)
	{
		UpdateFreeFlyInput();
	}
	else
	{
		UpdatePossessInput(deltaSeconds);
	}

	Actor* controledActor = GetActor();
	Mat44 playerOrientation = controledActor->m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	g_theAudio->UpdateListener(m_playerIndex, controledActor->m_position, playerOrientation.GetIBasis3D(), playerOrientation.GetKBasis3D());
	UpdateGradeText();
}

void Player::HandleFreeFlyKeyboard()
{
	float systemDelta = Clock::GetSystemClock().GetDeltaSeconds();
	IntVec2 cursorDelta = g_theInput->GetCursorClientDelta();
	Vec2 rotationDelta((float)cursorDelta.x * m_mouseLookSensitivity, (float)cursorDelta.y * m_mouseLookSensitivity);
	m_worldCamera.m_orientation.m_yaw -= rotationDelta.x;
	m_worldCamera.m_orientation.m_pitch -= rotationDelta.y;
	
	m_worldCamera.m_orientation.m_pitch = Clamp(m_worldCamera.m_orientation.m_pitch, -85.f, 85.f);

	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	m_worldCamera.m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

	if (g_theInput->IsKeyDown(KEYCODE_L_SHIFT))
	{
		m_currentMoveSpeed = m_sprintMoveSpeed;
	}
	else
	{
		m_currentMoveSpeed = m_defaultMoveSpeed;
	}
	if (g_theInput->IsKeyDown('W'))
	{
		m_worldCamera.m_position += forward * systemDelta * m_currentMoveSpeed;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_worldCamera.m_position -= forward * systemDelta * m_currentMoveSpeed;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_worldCamera.m_position += left * systemDelta * m_currentMoveSpeed;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_worldCamera.m_position -= left * systemDelta * m_currentMoveSpeed;
	}
	if (g_theInput->IsKeyDown('Z'))
	{
		m_worldCamera.m_position += Vec3(0.f, 0.f, 1.f) * systemDelta * m_currentMoveSpeed;
	}
	if (g_theInput->IsKeyDown('C'))
	{
		m_worldCamera.m_position -= Vec3(0.f, 0.f, 1.f) * systemDelta * m_currentMoveSpeed;
	}
}

void Player::UpdatePossessInput(float deltaSeconds)
{
	if (m_controllerIndex != -1 && g_theInput->GetController(m_controllerIndex).IsConnected())
	{
		HandlePossessController(deltaSeconds);
	}
	else
	{
		HandlePossessKeyboard(deltaSeconds);
	}
}

void Player::UpdateFreeFlyInput()
{
	if (m_controllerIndex != -1 && g_theInput->GetController(m_controllerIndex).IsConnected())
	{
		HandleFreeFlyController();
	}
	else
	{
		HandleFreeFlyKeyboard();
	}
}

void Player::HandleFreeFlyController()
{
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();
	//rotation
	XboxController const controller =  g_theInput->GetController(m_controllerIndex);
	Vec2 rightJoystickPos = controller.GetRightStick().GetPosition();
	m_worldCamera.m_orientation.m_yaw -= rightJoystickPos.x * deltaSeconds * m_controllerLookSensitivity;
	m_worldCamera.m_orientation.m_pitch -= rightJoystickPos.y * deltaSeconds * m_controllerLookSensitivity;
	m_worldCamera.m_orientation.m_pitch = Clamp(m_worldCamera.m_orientation.m_pitch, -85.f, 85.f);

	//sprint
	if (controller.IsButtonDown(XboxController::A_BUTTON))
	{
		m_currentMoveSpeed = m_sprintMoveSpeed;
	}
	else
	{
		m_currentMoveSpeed = m_defaultMoveSpeed;
	}

	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	m_worldCamera.m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
	Vec2 leftJoystickPos = controller.GetLeftStick().GetPosition();

	m_worldCamera.m_position += leftJoystickPos.y * forward * deltaSeconds * m_currentMoveSpeed;
	m_worldCamera.m_position -= leftJoystickPos.x * left * deltaSeconds * m_currentMoveSpeed;

	if (controller.IsButtonDown(XboxController::LB_BUTTON))
	{
		m_worldCamera.m_position -= Vec3(0.f, 0.f, 1.f) * deltaSeconds * m_currentMoveSpeed;
	}
	if (controller.IsButtonDown(XboxController::RB_BUTTON))
	{
		m_worldCamera.m_position += Vec3(0.f, 0.f, 1.f) * deltaSeconds * m_currentMoveSpeed;
	}
	if (controller.WasButtonJustPressed(XboxController::START_BUTTON))
	{
		m_worldCamera.m_position = Vec3::ZERO;
		m_worldCamera.m_orientation.m_pitch = 0.f;
		m_worldCamera.m_orientation.m_roll = 0.f;
		m_worldCamera.m_orientation.m_yaw = 0.f;
	}
}

void Player::SetNormalizedViewport(AABB2 const& normalizedViewport)
{
	m_worldCamera.m_normalizedViewport = normalizedViewport;
	m_screenCamera.m_normalizedViewport = normalizedViewport;
	Vec2 screenDimensions = GetScreenDimensions();
	Vec2 normalizedDimensions = m_screenCamera.m_normalizedViewport.GetDimensions();

	//take the normalized viewport and scale it up so that the smaller dimension is equal to 1
	//this way sprites will stay at a constant size and not get scaled
	Vec2 scalingDimensions = normalizedDimensions;
	float smallerDimension = normalizedDimensions.x < normalizedDimensions.y ? normalizedDimensions.x : normalizedDimensions.y;
	scalingDimensions.x /= smallerDimension;
	scalingDimensions.y /= smallerDimension;
	m_screenCamera.SetOrthographicView(Vec2::ZERO, screenDimensions * scalingDimensions);
}

void Player::OnKilledBy(ActorUID damageDealer)
{
	Actor* damageDealerPtr = damageDealer.GetActor();
	if (damageDealerPtr == nullptr || damageDealerPtr->m_controller == nullptr)
	{
		return;
	}

	Player* murderer = dynamic_cast<Player*>(damageDealerPtr->m_controller);
	
	if (murderer != nullptr)
	{
		murderer->m_kills++;
		m_deaths++;
	}
}

void Player::UpdateGradeText()
{
	if (m_startGradeReductionTimer.HasPeriodElapsed())
	{
		m_startGradeReductionTimer.Stop();
		m_scoreReduceTimer.Start();
	}
	if (m_scoreReduceTimer.HasPeriodElapsed())
	{
		g_theAudio->StartSound(g_theAudio->CreateOrGetSound("Data/Audio/LosePoint.wav"));
		m_scoreReduceTimer.Start();
		m_currentGrade--;
		m_pointsToReduce--;
		if (m_pointsToReduce <= 0)
		{
			m_scoreReduceTimer.Stop();
		}
	}

	std::string debuginfoString = Stringf("GRADE: %d", m_currentGrade);
	Vec2 screenDimensions = GetScreenDimensions();
	DebugAddScreenText(debuginfoString, Vec2(screenDimensions.x * .6f, 740.f), 50.f, Vec2(1.f, .5f), 0.f);
}

void Player::ReduceGrade(int pointsOff, std::string message, float lossTime)
{
	m_pointsToReduce = pointsOff;
	g_theAudio->StartSound(g_theAudio->CreateOrGetSound("Data/Audio/LosePointsBegin.wav"));
	m_startGradeReductionTimer.Start();
	m_scoreReduceTimer.m_period = lossTime / ((float)pointsOff);
}


void Player::UpdateCamera()
{
	//death camera handling
	Actor* rawActorPtr = GetActor();
	if (rawActorPtr && !rawActorPtr->IsAlive())
	{
		float deadNormalizedT = (rawActorPtr->m_currentDeadTime * 2.f) / rawActorPtr->m_actorDefinition->m_corpseLifetime;
		if (deadNormalizedT > 1.f)
		{
			deadNormalizedT = 1.f;
		}
		m_worldCamera.m_position = rawActorPtr->m_position;
		m_worldCamera.m_position.z = Lerp(rawActorPtr->m_actorDefinition->m_eyeHeight, 0.f, deadNormalizedT);
		return;
	}

	if (m_freeFlyCameraMode)
	{
		return;
	}

	//allign camera to actor
	if (rawActorPtr != nullptr)
	{
		m_worldCamera.m_orientation = rawActorPtr->m_orientation;
		m_worldCamera.m_position = rawActorPtr->m_position;
		m_worldCamera.m_position += Vec3(0.f, 0.f, rawActorPtr->m_actorDefinition->m_eyeHeight);
		m_worldCamera.SetFOV(rawActorPtr->m_actorDefinition->m_cameraFOV);
	}
}

void Player::HandlePossessController(float deltaSeconds)
{
	Actor* controlledActor = GetActor();
	if (controlledActor == nullptr)
	{
		return;
	}
	ActorDefinition const* def = controlledActor->m_actorDefinition;
	//rotation
	XboxController const controller = g_theInput->GetController(m_controllerIndex);
	Vec2 rightJoystickPos = controller.GetRightStick().GetPosition();
	//rotation
	controlledActor->m_orientation.m_yaw -= rightJoystickPos.x * deltaSeconds * m_controllerLookSensitivity;
	controlledActor->m_orientation.m_pitch -= rightJoystickPos.y * deltaSeconds * m_controllerLookSensitivity;
	controlledActor->m_orientation.m_pitch = Clamp(controlledActor->m_orientation.m_pitch, -85.f, 85.f);

	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	m_worldCamera.m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
	Vec2 leftJoystickPos = controller.GetLeftStick().GetPosition();

	Vec3 translateDir = leftJoystickPos.y * forward - leftJoystickPos.x * left;
	if (translateDir.GetLength() > 1.f)
	{
		translateDir = translateDir.GetNormalized();
	}

	if (controller.IsButtonDown(XboxController::A_BUTTON))
	{
		m_currentMoveSpeed = def->m_runSpeed;
	}
	else
	{
		m_currentMoveSpeed = def->m_walkSpeed;
	}

	if (translateDir.GetLength() > .01f)
	{
		controlledActor->MoveInDirection(translateDir, m_currentMoveSpeed * translateDir.GetLength());
	}

	if (controller.GetRightTrigger() > 0.f)
	{
		controlledActor->Attack();
	}

	if (controller.WasButtonJustPressed(XboxController::DPAD_UP_BUTTON))
	{
		controlledActor->EquipNextWeapon();
	}
	if (controller.WasButtonJustPressed(XboxController::DPAD_DOWN_BUTTON))
	{
		controlledActor->EquipPreviousWeapon();
	}
	if (controller.WasButtonJustPressed(XboxController::X_BUTTON))
	{
		controlledActor->m_currentWeaponIdx = 0;
	}
	if (controller.WasButtonJustPressed(XboxController::Y_BUTTON))
	{
		controlledActor->m_currentWeaponIdx = 1;
	}
	if (controller.WasButtonJustPressed(XboxController::A_BUTTON))
	{

		g_dialogSystem->AdvanceMessage();
	}

}

void Player::HandlePossessKeyboard(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	Actor* controlledActor = GetActor();
	if (controlledActor == nullptr)
	{
		return;
	}
	ActorDefinition const* def = controlledActor->m_actorDefinition;

	//rotation
	EulerAngles& cameraOrientation = controlledActor->m_orientation;
	IntVec2 cursorDelta = g_theInput->GetCursorClientDelta();
	Vec2 rotationDelta((float)cursorDelta.x * m_mouseLookSensitivity, (float)cursorDelta.y * m_mouseLookSensitivity);
	cameraOrientation.m_yaw -= rotationDelta.x;
	cameraOrientation.m_pitch -= rotationDelta.y;
	
	cameraOrientation.m_pitch = Clamp(cameraOrientation.m_pitch, -85.f, 85.f);

	//translation
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	controlledActor->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

	if (g_theInput->IsKeyDown(KEYCODE_L_SHIFT))
	{
		m_currentMoveSpeed = def->m_runSpeed;
	}
	else
	{
		m_currentMoveSpeed = def->m_walkSpeed;
	}
	if (g_theInput->IsKeyDown('W'))
	{
		controlledActor->MoveInDirection(forward, m_currentMoveSpeed);
	}
	if (g_theInput->IsKeyDown('S'))
	{
		controlledActor->MoveInDirection(-forward, m_currentMoveSpeed);
	}
	if (g_theInput->IsKeyDown('A'))
	{
		controlledActor->MoveInDirection(left, m_currentMoveSpeed);
	}
	if (g_theInput->IsKeyDown('D'))
	{
		controlledActor->MoveInDirection(-left, m_currentMoveSpeed);
	}

	if (g_theInput->WasKeyJustPressed('1'))
	{
		if (GetActor()->m_weapons.size() > 0)
		{
			GetActor()->m_currentWeaponIdx = 0;
		}
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		if (GetActor()->m_weapons.size() > 1)
		{
			GetActor()->m_currentWeaponIdx = 1;
		}
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		if (GetActor()->m_weapons.size() > 2)
		{
			GetActor()->m_currentWeaponIdx = 2;
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_ARROW))
	{
		GetActor()->EquipPreviousWeapon();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_ARROW))
	{
		GetActor()->EquipNextWeapon();
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		controlledActor->Attack();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		g_dialogSystem->AdvanceMessage();
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		g_dialogSystem->AdvanceSequence();
	}
	if (g_theInput->WasKeyJustPressed('M'))
	{
		g_theGame->CompleteCurrentTask();
	}
	if (g_theInput->WasKeyJustPressed('B'))
	{
		g_theGame->AdvanceMap();
	}




}