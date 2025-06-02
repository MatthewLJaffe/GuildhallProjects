#include "Pachinko2DVisualTest.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Window.hpp"

float fixedPhysicsTimeStep = .005f;
bool variablePhysicsTimestep = false;
float prevPhysicsTimestep = fixedPhysicsTimeStep;


Pachinkio2DVisualTest::Pachinkio2DVisualTest(VisualTestType myTestType, Game* game)
	: VisualTest(myTestType, game)
{
}

Pachinkio2DVisualTest::~Pachinkio2DVisualTest()
{
	for (size_t i = 0; i < m_bumpers.size(); i++)
	{
		delete m_bumpers[i];
	}
	m_bumpers.clear();
}

void Pachinkio2DVisualTest::InitializeTest()
{
	g_theInput->SetCursorMode(false, false);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_bitmapFont->AddVertsForText2D(m_textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .95f), 1.75f, "(Mode F6/F7 for prev/next): Pachinko Machine (2D)", Rgba8(255, 250, 100));
	RandomizeTest();
}

void Pachinkio2DVisualTest::RandomizeTest()
{
	fixedPhysicsTimeStep = .005f;
	variablePhysicsTimestep = false;
	m_mobileDiscs.clear();
	for (size_t i = 0; i < m_bumpers.size(); i++)
	{
		delete m_bumpers[i];
	}
	m_bumpers.clear();
	//add discs
	for (int i = 0; i < NUM_DISC_BUMPERS; i++)
	{
		m_bumpers.push_back(new DiscBumper(RollRandomPositionOnScreen(20.f), g_randGen->RollRandomFloatInRange(2.f, 4.f)));
	}

	//add capsules
	for (int i = 0; i < NUM_CAPSULE_BUMPERS; i++)
	{
		Vec2 boneStartPos = RollRandomPositionOnScreen(20.f);
		Vec2 boneEndPos = boneStartPos + g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(2.f, 4.f);
		m_bumpers.push_back(new CapsuleBumper(boneStartPos, boneEndPos, g_randGen->RollRandomFloatInRange(2.f, 4.f)));
	}

	//add obbs
	for (int i = 0; i < NUM_DISC_BUMPERS; i++)
	{
		OBB2 bumper;
		bumper.m_center = RollRandomPositionOnScreen(20.f);
		bumper.m_iBasisNormal = g_randGen->RollRandomNormalizedVec2();
		bumper.m_halfDimensions = Vec2(g_randGen->RollRandomFloatInRange(1.f, 2.f), g_randGen->RollRandomFloatInRange(2.f, 4.f));
		m_bumpers.push_back(new OBBBumper(bumper));
	}
}

void Pachinkio2DVisualTest::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || g_theInput->IsKeyDown('N'))
	{
		MobileDisc newDisc(m_arrowStartPos, m_arrowEndPos - m_arrowStartPos, g_randGen->RollRandomFloatInRange(MOBILE_DISC_MIN_SIZE, MOBILE_DISC_MAX_SIZE));
		m_mobileDiscs.push_back(newDisc);
	}
	if (g_theInput->WasKeyJustPressed('B'))
	{
		m_bottomWarp = !m_bottomWarp;
	}
	UpdateArrow(deltaSeconds);

	if (!variablePhysicsTimestep)
	{
		m_physicsTimeDebt += deltaSeconds;
		while (m_physicsTimeDebt > fixedPhysicsTimeStep)
		{

			UpdatePhysics();
			m_physicsTimeDebt -= fixedPhysicsTimeStep;
		}
	}
	else
	{
		fixedPhysicsTimeStep = deltaSeconds;
		UpdatePhysics();
	}
}

void Pachinkio2DVisualTest::UpdatePhysics()
{
	//move discs
	for (size_t i = 0; i < m_mobileDiscs.size(); i++)
	{
		m_mobileDiscs[i].m_velocity += Vec2::DOWN * GRAVITY * fixedPhysicsTimeStep;
		m_mobileDiscs[i].m_dir = m_mobileDiscs[i].m_velocity.GetNormalized();
		m_mobileDiscs[i].m_position += m_mobileDiscs[i].m_velocity * fixedPhysicsTimeStep;
		if (m_bottomWarp && m_mobileDiscs[i].m_position.y < -m_mobileDiscs[i].m_radius)
		{
			m_mobileDiscs[i].m_position.y = GetScreenHeight() * 1.1f + m_mobileDiscs[i].m_radius;
		}
	}

	//balls vs balls
	for (int currBallIdx = 0; currBallIdx < (int)m_mobileDiscs.size() - 1; currBallIdx++)
	{
		for (int otherBallIdx = currBallIdx + 1; otherBallIdx < (int)m_mobileDiscs.size(); otherBallIdx++)
		{
			MobileDisc& currBall = m_mobileDiscs[currBallIdx];
			MobileDisc& otherBall = m_mobileDiscs[otherBallIdx];
			float totalE = currBall.m_elasticity * otherBall.m_elasticity;
			BounceMobileDiscsOffEachOther2D(currBall.m_position, currBall.m_velocity, currBall.m_radius, otherBall.m_position, otherBall.m_velocity, otherBall.m_radius, totalE);
		}
	}

	//balls vs bumpers
	for (size_t bumperIdx = 0; bumperIdx < m_bumpers.size(); bumperIdx++)
	{
		for (size_t ballIdx = 0; ballIdx < m_mobileDiscs.size(); ballIdx++)
		{
			m_bumpers[bumperIdx]->BounceMobileDisc(m_mobileDiscs[ballIdx]);
		}
	}

	//balls vs walls
	for (size_t i = 0; i < m_mobileDiscs.size(); i++)
	{
		MobileDisc& currBall = m_mobileDiscs[i];
		if (currBall.m_position.x > GetScreenWidth() - currBall.m_radius)
		{
			currBall.m_position.x = GetScreenWidth() - currBall.m_radius;
			currBall.m_velocity.Reflect(Vec2::LEFT, currBall.m_elasticity);
		}
		else if (currBall.m_position.x < currBall.m_radius)
		{
			currBall.m_position.x = currBall.m_radius;
			currBall.m_velocity.Reflect(Vec2::RIGHT, currBall.m_elasticity);
		}
		if (!m_bottomWarp && currBall.m_position.y < currBall.m_radius)
		{
			currBall.m_position.y = currBall.m_radius;
			currBall.m_velocity.Reflect(Vec2::UP, currBall.m_elasticity);
		}
	}
}

void Pachinkio2DVisualTest::UpdateArrow(float deltaSeconds)
{
	//move arrow start
	Vec2 moveDir(0, 0);
	if (g_theInput->IsKeyDown('W'))
	{
		moveDir.y += 1.f;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveDir.x += -1.f;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveDir.y += -1.f;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveDir.x += 1.f;
	}
	if (moveDir != Vec2::ZERO)
	{
		moveDir.Normalize();
		m_arrowStartPos += moveDir * m_startMoveSpeed * deltaSeconds;
	}

	moveDir = Vec2::ZERO;
	if (g_theInput->IsKeyDown('I'))
	{
		moveDir.y += 1.f;
	}
	if (g_theInput->IsKeyDown('J'))
	{
		moveDir.x += -1.f;
	}
	if (g_theInput->IsKeyDown('K'))
	{
		moveDir.y += -1.f;
	}
	if (g_theInput->IsKeyDown('L'))
	{
		moveDir.x += 1.f;
	}
	if (moveDir != Vec2::ZERO)
	{
		moveDir.Normalize();
		m_arrowEndPos += moveDir * m_startMoveSpeed * deltaSeconds;
	}

	moveDir = Vec2::ZERO;
	if (g_theInput->IsKeyDown(KEYCODE_UP_ARROW))
	{
		moveDir.y += 1.f;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_ARROW))
	{
		moveDir.x += -1.f;
	}
	if (g_theInput->IsKeyDown(KEYCODE_DOWN_ARROW))
	{
		moveDir.y += -1.f;
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_ARROW))
	{
		moveDir.x += 1.f;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 normalizedMousePos = g_theWindow->GetNormalizedCursorPos();
		m_arrowStartPos = Vec2(normalizedMousePos.x * GetScreenWidth(), normalizedMousePos.y * GetScreenHeight());
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 normalizedMousePos = g_theWindow->GetNormalizedCursorPos();
		m_arrowEndPos = Vec2(normalizedMousePos.x * GetScreenWidth(), normalizedMousePos.y * GetScreenHeight());
	}
	if (g_theInput->WasKeyJustPressed('X'))
	{
		fixedPhysicsTimeStep *= .75f;
	}

	if (g_theInput->WasKeyJustPressed('C'))
	{
		fixedPhysicsTimeStep *= 1.25f;
	}

	if (g_theInput->WasKeyJustPressed('V'))
	{
		variablePhysicsTimestep = !variablePhysicsTimestep;
		if (variablePhysicsTimestep)
		{
			prevPhysicsTimestep = fixedPhysicsTimeStep;
		}
		else
		{
			fixedPhysicsTimeStep = prevPhysicsTimestep;
		}
	}


	moveDir.Normalize();
	Vec2 displacment = moveDir * m_arrowTranslateSpeed * deltaSeconds;
	m_arrowStartPos += displacment;
	m_arrowEndPos += displacment;
}

void Pachinkio2DVisualTest::Render()
{
	g_theRenderer->BeginCamera(m_game->m_screenCamera);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(nullptr);

	//draw bumpers
	for (size_t i = 0; i < m_bumpers.size(); i++)
	{
		m_bumpers[i]->Render();
	}

	//draw balls
	std::vector<Vertex_PCU> ballVerts;
	ballVerts.reserve(64 * m_mobileDiscs.size());
	for (size_t i = 0; i < m_mobileDiscs.size(); i++)
	{
		AddVertsForDisc2D(ballVerts, m_mobileDiscs[i].m_position, m_mobileDiscs[i].m_radius, m_mobileDiscs[i].m_color);
	}
	g_theRenderer->DrawVertexArray(ballVerts.size(), ballVerts.data());

	//draw arrow
	g_theRenderer->BindTexture(nullptr);
	std::vector<Vertex_PCU> arrowVerts;
	AddVertsForArrow2D(arrowVerts, m_arrowStartPos, m_arrowEndPos, 1.f, .1f, Rgba8::YELLOW);
	AddVertsForRing2D(arrowVerts, m_arrowStartPos, MOBILE_DISC_MIN_SIZE, .15f, Rgba8::YELLOW);
	AddVertsForRing2D(arrowVerts, m_arrowStartPos, MOBILE_DISC_MAX_SIZE, .15f, Rgba8::YELLOW);
	g_theRenderer->DrawVertexArray(arrowVerts.size(), arrowVerts.data());

	//draw text
	g_theRenderer->BindTexture(g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(m_textVerts.size(), m_textVerts.data());

	std::vector<Vertex_PCU> constrolsVerts;
	std::string bottomWrapOn = "Off";
	if (m_bottomWarp)
	{
		bottomWrapOn = "On";
	}
	float physicsMS = fixedPhysicsTimeStep * 1000.f;
	float deltaMS = g_theApp->GetGameClock()->GetDeltaSeconds() * 1000.f;
	std::string controlsText = Stringf("F8 to reset; LMB/RMB/WASD/IJKL move; T = slow, \nspace/N=ball(%d), B=bottom Wrap %s, timeStep=%.2fms (V,X,C), dt=%.1fms", 
		(int)m_mobileDiscs.size(), bottomWrapOn.c_str(), physicsMS, deltaMS);
	if (variablePhysicsTimestep)
	{
		controlsText = Stringf("F8 to reset; LMB/RMB/WASD/IJKL move; T = slow, \nspace/N=ball(%d), B=bottom Wrap %s, timeStep=variable (V,X,C), dt=%.1fms",
			(int)m_mobileDiscs.size(), bottomWrapOn.c_str(), deltaMS);
	}
	g_bitmapFont->AddVertsForTextInBox2D(constrolsVerts, AABB2(Vec2(0.f, GetScreenHeight()* .85f), Vec2(GetScreenWidth(), GetScreenHeight() * 1.f)), 1.75f,
		controlsText, Rgba8(100, 255, 200), 1.f, Vec2(.065f, .5f));
	g_theRenderer->DrawVertexArray(constrolsVerts.size(), constrolsVerts.data());
	g_theRenderer->EndCamera(m_game->m_screenCamera);
}

MobileDisc::MobileDisc(Vec2 pos, Vec2 veloctiy, float radius)
	: m_position(pos)
	, m_velocity(veloctiy)
	, m_radius(radius)
{
	m_color = LerpColor(LIGHT_BLUE, DARK_BLUE, g_randGen->RollRandomFloatInRange(0.f, 1.f)); 
}


Bumper::Bumper()
{
	m_elasticity = g_randGen->RollRandomFloatInRange(.1f, .9f);
	m_color = LerpColor(Rgba8::RED, Rgba8::GREEN, m_elasticity);
}

void Bumper::Render()
{
	g_theRenderer->DrawVertexArray(m_bumperVerts.size(), m_bumperVerts.data());
}

bool Bumper::BounceMobileDisc(MobileDisc& discToBounce)
{
	if (!DoDiscsOverlap(discToBounce.m_position, discToBounce.m_radius, m_boundingCirclePos, m_boundingCircleRadius))
	{
		return false;
	}
	Vec2 nearestBumperPos = GetNearestPointOnBumper(discToBounce.m_position);
	return BounceMobileDiscOffFixedPoint2D(discToBounce.m_position, discToBounce.m_velocity, discToBounce.m_radius, nearestBumperPos, discToBounce.m_elasticity * m_elasticity);
}

CapsuleBumper::CapsuleBumper(Vec2 boneStartPos, Vec2 boneEndPos, float boneRadius)
	: m_boneStartPos(boneStartPos)
	, m_boneEndPos(boneEndPos)
	, m_boneRadius(boneRadius)
{
	AddVertsForCapsule2D(m_bumperVerts, m_boneStartPos, m_boneEndPos, m_boneRadius, m_color);
	m_boundingCirclePos = (boneStartPos + boneEndPos) * .5f;
	m_boundingCircleRadius = (GetDistance2D(boneStartPos, boneEndPos) + 2*m_boneRadius) *.5f;
}

Vec2 CapsuleBumper::GetNearestPointOnBumper(Vec2 const& discPos) const
{
	return GetNearestPointOnCapsule2D(discPos, m_boneStartPos, m_boneEndPos, m_boneRadius);
}

DiscBumper::DiscBumper(Vec2 discPos, float discRadius)
	: m_discPos(discPos)
	, m_discRadius(discRadius)
{
	AddVertsForDisc2D(m_bumperVerts, m_discPos, m_discRadius, m_color);
	m_boundingCirclePos = m_discPos;
	m_boundingCircleRadius = m_discRadius;
}

Vec2 DiscBumper::GetNearestPointOnBumper(Vec2 const& point) const
{
	return GetNearestPointOnDisc2D(point, m_discPos, m_discRadius);
}

OBBBumper::OBBBumper(OBB2 obb)
	: m_box(obb)
{
	AddVertsForOOB2D(m_bumperVerts, m_box, m_color);
	m_boundingCirclePos = m_box.m_center;
	m_boundingCircleRadius = m_box.m_halfDimensions.GetLength();
}

Vec2 OBBBumper::GetNearestPointOnBumper(Vec2 const& discPos) const
{
	return GetNearestPointOnOBB2D(discPos, m_box);
}