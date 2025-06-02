#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

Game::Game(App* app)
	: m_theApp(app)
{}

Game::~Game() {}

void Game::StartUp()
{
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_attractScreen = true;
	m_player = new Player(this, Vec3::ZERO);
	m_allEntities.push_back(m_player);
	//AddColoredCube(Vec3(2.f, 2.f, 0.f), 1.f);
	//m_allEntities[1]->m_angularVelocity.m_roll = 30.f;
	//m_allEntities[1]->m_angularVelocity.m_pitch = 30.f;
	//AddColoredCube(Vec3(-2.f, -2.f, 0.f), 1.f);
	//AddSphere(Vec3(10.f, -5.f, 1.f), 1.f, Rgba8::WHITE);
	Vec3 point1 = Vec3(5.f, 5.f, 0.f);
	Vec3 point2 = Vec3(6.f, 3.f, -1.f);
	//AddCylinder(point1, point2, Rgba8::BLUE);
	//AddSphere(point1, .2f, Rgba8::WHITE);
	//AddSphere(point2, .2f, Rgba8::WHITE);
	AddGrid();
	font = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
	m_screenCamera.SetOrthographicView(Vec2::ZERO, Vec2(1600.f, 800.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_player->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .1f, 100.f);
	m_player->m_playerCamera.m_mode = Camera::eMode_Perspective;
	m_player->m_playerCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	m_player->m_position = Vec3(-2.f, 0.f, 0.f);
	m_player->m_playerCamera.m_position = m_player->m_position;

	m_randomRotationAxis = g_randGen->RollRandomNormalizedVec3();
}

void Game::StartGame()
{
	m_attractScreen = false;
	Mat44 xAxisTextTransform;
	xAxisTextTransform.AppendTranslation3D(Vec3(2.f, 0.f, .25f));
	DebugAddWorldText("X axis", xAxisTextTransform, .25f, Vec2(.5f, .5f), -1.f, Rgba8::RED, Rgba8::RED);

	Mat44 yAxisTextTransform;
	yAxisTextTransform.AppendTranslation3D(Vec3(0.f, 2.f, .25f));
	yAxisTextTransform.AppendZRotation(270.f);
	DebugAddWorldText("Y axis", yAxisTextTransform, .25f, Vec2(.5f, .5f), -1.f, Rgba8::GREEN, Rgba8::GREEN);

	Mat44 zAxisTextTransform;
	zAxisTextTransform.AppendTranslation3D(Vec3(0.f, 0.f, 2.f));
	zAxisTextTransform.AppendZRotation(-90.f);
	zAxisTextTransform.AppendYRotation(90.f);
	DebugAddWorldText("Z axis", zAxisTextTransform, .25f, Vec2(.5f, .5f), -1.f, Rgba8::BLUE, Rgba8::BLUE);

	Mat44 basisTransform;
	basisTransform.AppendTranslation3D(Vec3(0.f, 0.f, .25f));
	DebugAddWorldBasis(basisTransform, -1.f);
}

void Game::RenderPlaceHolderAttractScreen() const
{
	std::vector<Vertex_PCU> textVerts;
	AddVertsForTextTriangles2D(textVerts, "Protogame3D", Vec2(650.f, 380.f), 40.f, Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Game::RenderPlaceholderGameScreen() const
{
	std::vector<Vertex_PCU> ringVerts;
	ringVerts.reserve(3*32);
	AddVertsForRing2D(ringVerts, Vec2(100.f, 50.f), m_playRadius, 2.f, Rgba8(255, 0, 0, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(static_cast<int>(ringVerts.size()), ringVerts.data());

}

void Game::UpdatePlaceHolderAttractScreen()
{
	float currT = static_cast<float>(GetCurrentTimeSeconds());
	m_attractAlpha = RangeMap(sinf(currT * 2.f), -1.f, 1.f, .5f, 1.f);
}

void Game::UpdatePlaceHolderGameScreen(float deltaSeconds)
{
	m_currT += deltaSeconds;
	m_playRadius = RangeMap(sinf(m_currT), -1.f, 1.f, 10.f, 30.f);
}

void Game::AddColoredCube(Vec3 pos, float sideLength)
{
	Prop* prop = new Prop(this, pos);
	std::vector<Vertex_PCU> cubeVerts;
	Vec3 bottomLeftNear = Vec3(-.5f, .5f, -.5f) * sideLength;
	Vec3 bottomRightNear = Vec3(-.5f, -.5f, -.5f) * sideLength;
	Vec3 topRightNear = Vec3(-.5f, -.5f, .5f) * sideLength;
	Vec3 topLeftNear = Vec3(-.5f, .5f, .5f) * sideLength;

	Vec3 bottomLeftFar = Vec3(.5f, .5f, -.5f) * sideLength;
	Vec3 bottomRightFar = Vec3(.5f, -.5f, -.5f) * sideLength;
	Vec3 topRightFar = Vec3(.5f, -.5f, .5f) * sideLength;
	Vec3 topLeftFar = Vec3(.5f, .5f, .5f) * sideLength;	

	AddVertsForQuad3D(cubeVerts, bottomRightFar, bottomLeftFar, topLeftFar, topRightFar, Rgba8::RED);
	AddVertsForQuad3D(cubeVerts, bottomLeftNear, bottomRightNear, topRightNear, topLeftNear, Rgba8::CYAN);

	AddVertsForQuad3D(cubeVerts, topLeftNear, topRightNear, topRightFar, topLeftFar, Rgba8::BLUE);
	AddVertsForQuad3D(cubeVerts, bottomRightNear, bottomLeftNear, bottomLeftFar, bottomRightFar, Rgba8::YELLOW);

	AddVertsForQuad3D(cubeVerts, bottomLeftFar, bottomLeftNear, topLeftNear, topLeftFar, Rgba8::GREEN);
	AddVertsForQuad3D(cubeVerts, bottomRightNear, bottomRightFar, topRightFar, topRightNear, Rgba8::MAGENTA);

	prop->m_vertexes = cubeVerts;
	m_allEntities.push_back(prop);
}

void Game::AddAABB3D(AABB3 bounds, Rgba8 color)
{
	Prop* prop = new Prop(this, (bounds.m_mins + bounds.m_maxs) * .5f);
	std::vector<Vertex_PCU> aabb3Verts;
	AddVertsForAABB3D(aabb3Verts, bounds, color);
	prop->m_vertexes = aabb3Verts;
	prop->m_texture = m_testTexture;
	m_allEntities.push_back(prop);
}

void Game::AddSphere(Vec3 pos, float radius, Rgba8 color)
{
	Prop* prop = new Prop(this, pos);
	std::vector<Vertex_PCU> propVerts;
	AddVertsForSphere3D(propVerts, Vec3::ZERO, radius, color, AABB2::ZERO_TO_ONE, 16, 8);
	prop->m_vertexes = propVerts;
	prop->m_texture = m_testTexture;
	prop->m_angularVelocity.m_yaw = 45.f;
	m_allEntities.push_back(prop);
}

void Game::AddGrid()
{
	Prop* grid = new Prop(this, Vec3::ZERO);
	std::vector<Vertex_PCU> gridVerts;
	AABB3 gridLineBounds;
	gridLineBounds.m_mins.y = -50.f;
	gridLineBounds.m_maxs.y = 50.f;
	float defaultLineThickness = .025f;
	for (int x = -50; x < 50; x++)
	{
		Rgba8 color(100, 100, 100);
		float lineThickness = defaultLineThickness;
		if (x % 5 == 0)
		{
			color = Rgba8::GREEN;
			lineThickness = defaultLineThickness * 2.f;
		}
		gridLineBounds.m_mins.z = -lineThickness;
		gridLineBounds.m_maxs.z = lineThickness;

		gridLineBounds.m_mins.x = static_cast<float>(x) - lineThickness;
		gridLineBounds.m_maxs.x = static_cast<float>(x) + lineThickness;
		AddVertsForAABB3D(gridVerts, gridLineBounds, color);
	}

	gridLineBounds.m_mins.x = -50.f;
	gridLineBounds.m_maxs.x = 50.f;
	for (int y = -50; y < 50; y++)
	{
		Rgba8 color(100, 100, 100);
		float lineThickness = defaultLineThickness;
		if (y % 5 == 0)
		{
			color = Rgba8::RED;
			lineThickness = defaultLineThickness * 2.f;
		}
		gridLineBounds.m_mins.z = -lineThickness;
		gridLineBounds.m_maxs.z = lineThickness;

		gridLineBounds.m_mins.y = static_cast<float>(y) - lineThickness;
		gridLineBounds.m_maxs.y = static_cast<float>(y) + lineThickness;
		AddVertsForAABB3D(gridVerts, gridLineBounds, color);
	}
	grid->m_vertexes = gridVerts;
	m_allEntities.push_back(grid);
}

void Game::AddCylinder(Vec3 const& startPoint, Vec3 const& endPoint, Rgba8 color)
{
	UNUSED(color);
	Prop* prop = new Prop(this, Vec3::ZERO);
	std::vector<Vertex_PCU> propVerts;
	AddVertsForCone3D(propVerts, startPoint, endPoint, 1.f, Rgba8::BLUE, AABB2::ZERO_TO_ONE, 32);
	prop->m_vertexes = propVerts;
	//prop->m_texture = m_testTexture;
	m_allEntities.push_back(prop);
}

void Game::CheckForDebugCommands()
{
	if (g_theInput->WasKeyJustPressed('1'))
	{
		DebugAddWorldLine(m_player->m_position, m_player->m_position + m_player->GetForwardNormal() * 20.f, .05f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
	}
	if (g_theInput->IsKeyDown('2'))
	{
		DebugAddWorldPoint(Vec3(m_player->m_position.x, m_player->m_position.y, 0.f), .25f, 60.f, Rgba8(150, 75, 0), Rgba8(150, 75, 0), DebugRenderMode::USE_DEPTH);
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		DebugAddWorldWireSphere(m_player->m_position + m_player->GetForwardNormal() * 2.f, 1.f, 5.f, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::USE_DEPTH);
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		Mat44 playerTrans;
		playerTrans.AppendTranslation3D(m_player->m_position);
		playerTrans.Append(m_player->m_orientationDegrees.GetAsMatrix_IFwd_JLeft_KUp());
		DebugAddWorldBasis(playerTrans, 20.f);
		//DebugAddWorldArrow(m_player->m_position, m_player->m_position + playerOrientation.GetIBasis3D(), .05f, 20.f, Rgba8::RED, Rgba8::RED);
		//DebugAddWorldArrow(m_player->m_position, m_player->m_position + playerOrientation.GetJBasis3D(), .05f, 20.f, Rgba8::GREEN, Rgba8::GREEN);
		//DebugAddWorldArrow(m_player->m_position, m_player->m_position + playerOrientation.GetKBasis3D(), .05f, 20.f, Rgba8::BLUE, Rgba8::BLUE);
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		std::string outputString = Stringf("Position: %.1f, %.1f, %.1f Orientation: %.1f, %.1f, %.1f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z, 
			m_player->m_orientationDegrees.m_yaw, m_player->m_orientationDegrees.m_pitch, m_player->m_orientationDegrees.m_roll);
		DebugAddWorldBillboardText(outputString, m_player->m_position, .1f, Vec2(.5f, .5f), 10.f, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH);
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		DebugAddWorldWireCylinder(m_player->m_position, m_player->m_position + Vec3(0.f, 0.f, 2.f), .5f, 10.f, Rgba8::WHITE, Rgba8::RED);
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		std::string outputString = Stringf("Orientation: %.1f, %.1f, %.1f",
			m_player->m_orientationDegrees.m_yaw, m_player->m_orientationDegrees.m_pitch, m_player->m_orientationDegrees.m_roll);
		DebugAddMessage(outputString, 20.f, 5.f);
	}
}

void Game::Update(float deltaSeconds)
{
	if (m_attractScreen)
	{
		UpdatePlaceHolderAttractScreen();
		//check for start game
		if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N'))
		{
			StartGame();
		}
		const XboxController controller = g_theInput->GetController(0);
		if (controller.IsConnected() && controller.WasButtonJustPressed(XboxController::A_BUTTON))
		{
			StartGame();
		}
	}
	else
	{
		if (g_theInput->WasKeyJustPressed('R'))
		{
			m_randomRotationAxis = g_randGen->RollRandomNormalizedVec3();
			m_testMat44 = Mat44();
		}

		float rotationRate = 45.f;
		m_testMat44.SetTranslation3D(m_arrowPosition);
		
		Mat44 rotationThisFrame = Mat44::CreateAxisAngleRotation(m_randomRotationAxis, rotationRate * deltaSeconds);
		m_testMat44.Append(rotationThisFrame);


		for (size_t i = 0; i < m_allEntities.size(); i++)
		{
			m_allEntities[i]->Update(deltaSeconds);
		}
		std::string playerPosStr = Stringf("Player Position: %.1f, %.1f, %.1f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z);
		DebugAddMessage(playerPosStr, 20.f, 0.f);
		std::string debuginfoString = Stringf("Time %.2f FPS:%.1f Scale: 1.0", Clock::GetSystemClock().GetTotalSeconds(), 1.f / g_theApp->m_clock->GetDeltaSeconds());
		DebugAddScreenText(debuginfoString, Vec2(900.f, 780.f), 20.f, Vec2(1.f, .5f), 0.f);

		CheckForDebugCommands();
		//float cubeColorValue = RangeMap(sinf(Clock::GetSystemClock().GetTotalSeconds()), -1.f, 1.f, 0.f, 1.f);
		//m_allEntities[2]->m_color = LerpColor(Rgba8::BLACK, Rgba8::WHITE, cubeColorValue);
	}
}

void Game::Render() const 
{
	if (m_attractScreen)
	{
		g_theRenderer->BeginCamera(m_screenCamera);
		RenderPlaceHolderAttractScreen();
		g_theRenderer->EndCamera(m_screenCamera);
	}
	else
	{
		//World Space Rendering
		g_theRenderer->BeginCamera(m_player->m_playerCamera);

		//test rotation
		std::vector<Vertex_PCU> arrowVerts;
		AddVertsForArrow3D(arrowVerts, m_arrowPosition, m_arrowPosition + m_randomRotationAxis * 2.f, .1f, Rgba8::CYAN);

		std::vector<Vertex_PCU> basisVerts;
		AddVertsForBasis3D(basisVerts, Mat44());
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetModelConstants(m_testMat44);
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
		g_theRenderer->DrawVertexArray(basisVerts.size(), basisVerts.data());

		g_theRenderer->SetModelConstants(Mat44());
		g_theRenderer->DrawVertexArray(arrowVerts.size(), arrowVerts.data());

		for (size_t i = 0; i < m_allEntities.size(); i++)
		{
			m_allEntities[i]->Render();
		}
		g_theRenderer->EndCamera(m_player->m_playerCamera);
		DebugRenderWorld(m_player->m_playerCamera);

		//ScreenSpace Rendering
		DebugRenderScreen(m_screenCamera);
	}
}

void Game::ShutDown()
{
	
}

bool Game::IsAttractScreen()
{
	return m_attractScreen;
}


