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
//#include "Engine/Math/Mat44.hpp"

Game::Game(App* app)
	: m_theApp(app)
{
	m_worldCamera.m_mode = Camera::eMode_Orthographic;
	m_worldCamera.SetOrthographicView(Vec2::ZERO, Vec2(200.f, 100.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2::ZERO,  Vec2(1600.f, 800.f));
}

Game::~Game() {}

void Game::StartUp()
{
	m_attractScreen = true;
	
	//Play Button
	m_attractPlayVerts[0] = Vertex_PCU(Vec3(1, 0, 0), Rgba8(61, 186, 86, 255));
	m_attractPlayVerts[1] = Vertex_PCU(Vec3(-1, 1, 0), Rgba8(61, 186, 86, 255));
	m_attractPlayVerts[2] = Vertex_PCU(Vec3(-1, -1, 0), Rgba8(61, 186, 86, 255));

	m_placeholderGameverts[0] = Vertex_PCU(Vec3(-1.f, 1.f, 0.f), Rgba8(0, 0, 255, 0));
	m_placeholderGameverts[1] = Vertex_PCU(Vec3(-1.f, -1.f, 0.f), Rgba8(0, 0, 255, 255));
	m_placeholderGameverts[2] = Vertex_PCU(Vec3(1.f, -1.f, 0.f), Rgba8(0, 0, 255, 255));
	
	m_placeholderGameverts[3] = Vertex_PCU(Vec3(1.f, 1.f, 0.f), Rgba8(0, 0, 255, 255));
	m_placeholderGameverts[4] = Vertex_PCU(Vec3(1.f, -1.f, 0.f), Rgba8(0, 0, 255, 255));
	m_placeholderGameverts[5] = Vertex_PCU(Vec3(-1.f, 1.f, 0.f), Rgba8(0, 0, 255, 0));

	SOUND_ID_TEST = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
	g_theAudio->StartSound(SOUND_ID_TEST);

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Test line");
}

void Game::StartGame()
{
	m_attractScreen = false;
}

void Game::RenderPlaceHolderAttractScreen() const
{
	std::vector<Vertex_PCU> textVerts;
	AddVertsForTextTriangles2D(textVerts, "Protogame2D", Vec2(650.f, 380.f), 40.f, Rgba8::WHITE);
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
		UpdatePlaceHolderGameScreen(deltaSeconds);
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
		g_theRenderer->BeginCamera(m_worldCamera);
		RenderPlaceholderGameScreen();
		g_theRenderer->EndCamera(m_worldCamera);

		//ScreenSpace Rendering
		g_theRenderer->BeginCamera(m_screenCamera);
		//Render stuff here
		g_theRenderer->EndCamera(m_screenCamera);
	}
}

void Game::ShutDown()
{
	
}

bool Game::IsAttractScreen()
{
	return m_attractScreen;
}


