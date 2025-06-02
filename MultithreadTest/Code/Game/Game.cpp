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
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)

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
	AddVertsForTextTriangles2D(textVerts, "MultithreadTest", Vec2(650.f, 380.f), 40.f, Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)textVerts.size(), textVerts.data());
}

void Game::RenderPlaceholderGameScreen() const
{
	Vec2 worldScreenDimensions = m_worldCamera.GetOrthoDimensions();
	Vec2 cellDimensions = Vec2( worldScreenDimensions.x / (40.f), worldScreenDimensions.y / (20.f) );
	float cellPadding = .05f;
	for (int i = 0; i < (int)m_testJobs.size(); i++)
	{
		IntVec2 testCellCoords(i % 40, i / 40);
		Vec2 cellTopLeft(m_worldCamera.GetOrthoBottomLeft().x + ((float)testCellCoords.x * cellDimensions.x), m_worldCamera.GetOrthoTopRight().y - ((float)testCellCoords.y * cellDimensions.y));
		AABB2 testCell(	Vec2(cellTopLeft.x + cellDimensions.x * cellPadding, cellTopLeft.y - cellDimensions.y * (1.f - cellPadding)), 
						Vec2(cellTopLeft.x + cellDimensions.x * (1.f - cellPadding) , cellTopLeft.y - cellDimensions.y * cellPadding));
		std::vector<Vertex_PCU> cellVerts;

		Rgba8 cellColor = Rgba8::RED;
		switch (m_testJobs[i]->m_jobStatus)
		{
		case JobStatus::CREATED:
			cellColor = Rgba8::BLACK;
			break;
		case JobStatus::QUEUED:
			cellColor = Rgba8::RED;
			break;
		case JobStatus::CLAIMED:
			cellColor = Rgba8::YELLOW;
			break;
		case JobStatus::COMPLETED:
			cellColor = Rgba8::GREEN;
			break;
		case JobStatus::RETRIEVED:
			cellColor = Rgba8::BLUE;
			break;
		}

		AddVertsForAABB2D(cellVerts, testCell, cellColor);

		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->BindTexture(nullptr);

		g_theRenderer->DrawVertexArray(cellVerts.size(), cellVerts.data());
	}
}

void Game::UpdatePlaceHolderAttractScreen()
{
	float currT = static_cast<float>(GetCurrentTimeSeconds());
	m_attractAlpha = RangeMap(sinf(currT * 2.f), -1.f, 1.f, .5f, 1.f);
}

void Game::UpdatePlaceHolderGameScreen(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (g_theInput->WasKeyJustPressed('J'))
	{
		TestJob* testJob = new TestJob();
		m_testJobs.push_back(testJob);
		g_theJobSystem->QueueJob(testJob);
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		std::vector<Job*> jobsToAdd;
		for (int i = 0; i < 100; i++)
		{
			TestJob* testJob = new TestJob();
			m_testJobs.push_back(testJob);
			jobsToAdd.push_back(testJob);
		}
		g_theJobSystem->QueueJobs(jobsToAdd);
	}

	if (g_theInput->WasKeyJustPressed('R'))
	{
		g_theJobSystem->GetCompletedJob();
	}
	if (g_theInput->WasKeyJustPressed('A'))
	{
		std::vector<Job*> jobsToRetrieve;
		g_theJobSystem->GetCompletedJobs(jobsToRetrieve);
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
	for (int i = 0; i < m_testJobs.size(); i++)
	{
		//delete m_testJobs[i];
		//m_testJobs[i] = nullptr;
	}
}

bool Game::IsAttractScreen()
{
	return m_attractScreen;
}

TestJob::TestJob()
{
	m_sleepTime = g_randGen->RollRandomIntInRange(50, 3000);
}

void TestJob::Execute()
{
	Sleep((DWORD)m_sleepTime);
}
