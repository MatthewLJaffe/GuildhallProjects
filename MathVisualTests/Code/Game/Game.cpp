#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Game/A5VisualTest.hpp"
#include "Game/A6VisualTest.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/RaycastVsLineSegmentTest.hpp"
#include "Game/RaycastVsAABB2DTest.hpp"
#include "Game/ShapesAndQueries3DTest.hpp"
#include "Game/EasingCurvesAndSplinesTest.hpp"
#include "Game/Pachinko2DVisualTest.hpp"
#include "Game/DrawingSpline.hpp"
#include "Game/ConvexSceneEditorTest.hpp"

Game::Game(App* app)
	: m_theApp(app)
{
	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2::ZERO, Vec2(200.f, 100.f));
}

Game::~Game() {}

void Game::StartUp()
{
	LoadAssets();
	g_bitmapFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Images/SquirrelFixedFont");
	m_visualTests.push_back(new A5VisualTest(VisualTestType::VISUAL_TEST_A5, this));
	m_visualTests.push_back(new A6VisualTest(VisualTestType::VISUAL_TEST_A6, this));
	m_visualTests.push_back(new RaycastVsLineSegmentTest(VisualTestType::RAYCAST_VS_LINESEG2D, this));
	m_visualTests.push_back(new RaycastVsAABB2DTest(VisualTestType::RAYCAST_VS_AABB2D, this));
	m_visualTests.push_back(new ShapesAndQueries3DTest(VisualTestType::SHAPES_AND_QUERIES_3D, this));
	m_visualTests.push_back(new EasingCurvesAndSplinesTest(VisualTestType::EASING_CURVES_AND_SPLINES, this));
	m_visualTests.push_back(new Pachinkio2DVisualTest(VisualTestType::PACHINKO_2D, this));
	m_visualTests.push_back(new DrawingSpline(VisualTestType::DRAWING_SPLINE, this));
	m_visualTests.push_back(new ConvexSceneEditorTest(VisualTestType::CONVEX_SCENE_EDITOR, this));

	for (int i = 0; i < (int)VisualTestType::NUM_VISUAL_TESTS; i++)
	{
		m_visualTests[i]->InitializeTest();
	}
	m_visualTestType = CONVEX_SCENE_EDITOR;
}

void Game::Update(float deltaSeconds)
{
	m_visualTests[(int)m_visualTestType]->Update(deltaSeconds);
}

void Game::Render() const
{
	m_visualTests[(int)m_visualTestType]->Render();
}

void Game::ShutDown()
{
	FloatCurve::ClearCurves();
	for (size_t i = 0; i < m_visualTests.size(); i++)
	{
		delete m_visualTests[i];
		m_visualTests[i] = nullptr;
	}
}

void Game::LoadAssets()
{
	FloatCurve::LoadCurvesFromXML("Data/Curves/TestCurve.xml");
}

VisualTest* Game::GetCurrentVisualTest()
{
	return m_visualTests[(int)m_visualTestType];
}

