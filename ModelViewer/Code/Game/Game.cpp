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
#include "Engine/Renderer/Material.hpp"
#include "Game/Prop.hpp"

Game::Game(App* app)
	: m_theApp(app)
{}

Game::~Game() {}

void Game::StartUp()
{
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_grid = new Prop(this, Vec3::ZERO);
	m_grid->CreateGrid();
	m_model = new Model(this, Vec3::ZERO);

	m_player = new Player(this, Vec3::ZERO);
	Scene* scene0 = new Scene();
	Model* playerModel = new Model(this, Vec3(-1.f, 0.f, 1.f));
	playerModel->Load("Data/Models/Quaternius/StrikerBlueModel.xml");
	scene0->m_entities.push_back(playerModel);

	m_woofer1 = new Model(this, Vec3(.725f, 0.f, 1.125f));
	m_woofer1->Load("Data/Models/Fancy/WooferModel.xml");
	scene0->m_entities.push_back(m_woofer1);
	m_woofer2 = new Model(this, Vec3(.2f, -.5f, .9f));
	m_woofer2->Load("Data/Models/Fancy/WooferModel.xml");
	scene0->m_entities.push_back(m_woofer2);
	m_woofer3 = new Model(this, Vec3(.2f, .5f, .9f));
	m_woofer3->Load("Data/Models/Fancy/WooferModel.xml");
	scene0->m_entities.push_back(m_woofer3);

	m_scenes.push_back(scene0);

	Scene* scene1 = new Scene();
	Prop* testCube = new Prop(this, Vec3(-2.f, 2.f, 0.f));
	testCube->m_material = new Material("Data/Materials/Grass.xml", g_theRenderer);
	testCube->CreateCube(1.5f);
	scene1->m_entities.push_back(testCube);

	Prop* testCube2 = new Prop(this, Vec3(2.f, -2.f, 0.f));
	testCube2->m_material = new Material("Data/Materials/Brick.xml", g_theRenderer);
	testCube2->CreateCube(1.5f);
	scene1->m_entities.push_back(testCube2);
	
	Prop* testSphere = new Prop(this, Vec3(2.f, 2.f, 0.f));
	testSphere->m_material = new Material("Data/Materials/Brick.xml", g_theRenderer);
	testSphere->CreateSphere(.75f);
	scene1->m_entities.push_back(testSphere);
	
	Prop* testSphere2 = new Prop(this, Vec3(-2.f, -2.f, 0.f));
	testSphere2->m_material = new Material("Data/Materials/Grass.xml", g_theRenderer);
	testSphere2->CreateSphere(.75f);
	scene1->m_entities.push_back(testSphere2);
	
	m_scenes.push_back(scene1);

	Scene* scene2 = new Scene();
	Model* tutorialBox = new Model(this, Vec3::ZERO);
	tutorialBox->Load("Data/Models/Tutorial_Box.xml");
	scene2->m_entities.push_back(tutorialBox);
	m_scenes.push_back(scene2);

	Scene* scene3 = new Scene();
	Model* tank = new Model(this, Vec3::ZERO);
	tank->Load("Data/Models/Hadrian.xml");
	scene3->m_entities.push_back(tank);
	m_scenes.push_back(scene3);

	font = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
	m_screenCamera.SetOrthographicView(Vec2::ZERO, Vec2(1600.f, 800.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_player->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .01f, 100.f);
	m_player->m_playerCamera.m_mode = Camera::eMode_Perspective;
	m_player->m_playerCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	m_player->m_position = Vec3(3.f, 0.f, 3.f);
	m_player->m_playerCamera.m_position = m_player->m_position;
	m_player->m_orientationDegrees = EulerAngles(180.f, 45.f, 0.f);

	Mat44 cpuMeshTransform;
	cpuMeshTransform.AppendScaleNonUniform3D(Vec3(.1f, .1f, .1f));
}

void Game::StartGame()
{
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

void Game::CheckForDebugCommands(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('L'))
	{
		std::string objFilepath = g_theWindow->GetFileFromWindowsExplorer();
		g_theInput->HandleKeyReleased('L');
		if (objFilepath != "")
		{
			Strings filePathSplit = SplitStringOnDelimiter(objFilepath, '.', true);
			if (filePathSplit[1] != "obj" && filePathSplit[1] != "xml")
			{
				ERROR_RECOVERABLE("Invalid file pick an obj file");
			}
			else
			{	
				if (m_model->Load(objFilepath))
				{
					m_showModel = true;
				}
			}
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_ARROW_BRACKET))
	{
		m_sunIntensity -= .1f;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_ARROW_BRACKET))
	{
		m_sunIntensity += .1f;
	}
	m_sunIntensity = Clamp(m_sunIntensity, 0.f, 1.f);
	if (g_theInput->WasKeyJustPressed('1'))
	{
		m_useAmbient = !m_useAmbient;
	}
	if (g_theInput->WasKeyJustPressed('2'))
	{
		m_useDiffuse = !m_useDiffuse;
	}
	if (g_theInput->WasKeyJustPressed('3'))
	{
		m_useSpecular = !m_useSpecular;
	}
	if (g_theInput->WasKeyJustPressed('4'))
	{
		m_useEmissive = !m_useEmissive;
	}
	if (g_theInput->WasKeyJustPressed('5'))
	{
		m_diffuseMap = !m_diffuseMap;
	}
	if (g_theInput->WasKeyJustPressed('6'))
	{
		m_normalMap = !m_normalMap;
	}
	if (g_theInput->WasKeyJustPressed('7'))
	{
		m_specularMap = !m_specularMap;
	}
	if (g_theInput->WasKeyJustPressed('8'))
	{
		m_glossinessMap = !m_glossinessMap;
	}
	if (g_theInput->WasKeyJustPressed('9'))
	{
		m_emissiveMap = !m_emissiveMap;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_SQUARE_BRACKET))
	{
		m_sceneIdx++;
		if (m_sceneIdx >= m_scenes.size())
		{
			m_sceneIdx = 0;
		}
		m_showModel = false;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_SQUARE_BRACKET))
	{
		m_sceneIdx--;
		if (m_sceneIdx < 0)
		{
			m_sceneIdx = (int)m_scenes.size() - 1;
		}
		m_showModel = false;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_ARROW))
	{
		m_sunOrientaiton.m_yaw -= 90.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_ARROW))
	{
		m_sunOrientaiton.m_yaw += 90.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEYCODE_DOWN_ARROW))
	{
		m_sunOrientaiton.m_pitch += 45.f * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEYCODE_UP_ARROW))
	{
		m_sunOrientaiton.m_pitch -= 45.f * deltaSeconds;
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		m_renderDebug = !m_renderDebug;
	}
	if (g_theInput->WasKeyJustPressed('R'))
	{
		m_debugRotation = !m_debugRotation;
	}
}

void Game::Update(float deltaSeconds)
{
	static float time = 0.f;
	time += deltaSeconds;
	CheckForDebugCommands(deltaSeconds);
	m_player->Update(deltaSeconds);
	if (m_showModel)
	{
		m_model->Update(deltaSeconds);
	}
	else
	{
		for (size_t i = 0; i < m_scenes[m_sceneIdx]->m_entities.size(); i++)
		{
			m_scenes[m_sceneIdx]->m_entities[i]->Update(deltaSeconds);
		}
	}
	float t = .5f * SinDegrees(time * 360.f) + .5f;
	m_woofer1->m_scale = Vec3::Lerp(Vec3(.9f, .9f, .9f), Vec3(1.1f, 1.1f, 1.1f), t);
	m_woofer2->m_scale = Vec3::Lerp(Vec3(.9f, .9f, .9f), Vec3(1.1f, 1.1f, 1.1f), t);
	m_woofer3->m_scale = Vec3::Lerp(Vec3(.9f, .9f, .9f), Vec3(1.1f, 1.1f, 1.1f), t);

	//AddDebugText();
	//DebugAddWorldArrow(Vec3(0.f, 0.f, 5.f), Vec3(0.f, 0.f, 5.f) + m_sunOrientaiton.GetIFwd() * 2.f, .25f, 0.f, Rgba8::YELLOW, Rgba8::YELLOW);
}

void Game::AddDebugText()
{
	std::string debuginfoString = Stringf("FPS: %.1f", 1.f / g_theApp->m_clock->GetDeltaSeconds());
	DebugAddMessage(debuginfoString, 20.f, 0.f);
	DebugAddMessage(Stringf("Sun Orientation (ARROWS): (%.1f, %.1f, %.1f)", m_sunOrientaiton.m_yaw, m_sunOrientaiton.m_pitch, m_sunOrientaiton.m_roll), 20.f, 0.f);
	DebugAddMessage(Stringf("Scene ([/]): %d", m_sceneIdx), 20.f, 0.f);
	Vec3 sunDirection = m_sunOrientaiton.GetIFwd();
	DebugAddMessage(Stringf("Sun Direction: (%.1f, %.1f, %.1f)", sunDirection.x, sunDirection.y, sunDirection.z), 20.f, 0.f);
	DebugAddMessage(Stringf("Sun Intensity: (</>): %.1f", m_sunIntensity), 20.f, 0.f);
	DebugAddMessage(Stringf("Ambient Intensity: (</>): %.1f", 1.f - m_sunIntensity), 20.f, 0.f);

	std::string ambientStr = Stringf("Ambient\t\t\t\t\t\t\t\t[1]: ");
	ambientStr += m_useAmbient ? "ON" : "OFF";
	DebugAddMessage(ambientStr, 20.f, 0.f);

	std::string diffuseStr = Stringf("Diffuse\t\t\t\t\t\t\t\t[2]: ");
	diffuseStr += m_useDiffuse ? "ON" : "OFF";
	DebugAddMessage(diffuseStr, 20.f, 0.f);

	std::string specularStr = Stringf("Specular\t\t\t\t\t\t\t[3]: ");
	specularStr += m_useSpecular ? "ON" : "OFF";
	DebugAddMessage(specularStr, 20.f, 0.f);

	std::string emissiveStr = Stringf("Emissive\t\t\t\t\t\t\t[4]: ");
	emissiveStr += m_useEmissive ? "ON" : "OFF";
	DebugAddMessage(emissiveStr, 20.f, 0.f);

	std::string dmapStr = Stringf("Diffuse map\t\t\t\t[5]: ");
	dmapStr += m_diffuseMap ? "ON" : "OFF";
	DebugAddMessage(dmapStr, 20.f, 0.f);

	std::string nMapStr = Stringf("Normal Map\t\t\t\t\t[6]: ");
	nMapStr += m_normalMap ? "ON" : "OFF";
	DebugAddMessage(nMapStr, 20.f, 0.f);

	std::string sMapStr = Stringf("Specular Map\t\t\t[7]: ");
	sMapStr += m_specularMap ? "ON" : "OFF";
	DebugAddMessage(sMapStr, 20.f, 0.f);

	std::string glossinessStr = Stringf("Glossiness Map\t[8]: ");
	glossinessStr += m_glossinessMap ? "ON" : "OFF";
	DebugAddMessage(glossinessStr, 20.f, 0.f);

	std::string eMap = Stringf("Emissive Map\t\t\t[9]: ");
	eMap += m_emissiveMap ? "ON" : "OFF";
	DebugAddMessage(eMap, 20.f, 0.f);
}



void Game::Render() const 
{
	//World Space Rendering
	g_theRenderer->BeginCamera(m_player->m_playerCamera);
	LightConstants lightConstants;
	lightConstants.AmbientIntensity = 1.f - m_sunIntensity;
	lightConstants.renderAmbientDebugFlag = (int)m_useAmbient;
	lightConstants.renderDiffuseDebugFlag = (int)m_useDiffuse;
	lightConstants.renderEmissiveDebugFlag = (int)m_useEmissive;
	lightConstants.renderSpecularDebugFlag = (int)m_useSpecular;
	lightConstants.SunDirection = m_sunOrientaiton.GetIFwd();
	lightConstants.SunIntensity = m_sunIntensity;
	lightConstants.useDiffuseMapDebugFlag = (int)m_diffuseMap;
	lightConstants.useEmissiveMapDebugFlag = (int)m_emissiveMap;
	lightConstants.useGlossinessMapDebugFlag = (int)m_glossinessMap;
	lightConstants.useNormalMapDebugFlag = (int)m_normalMap;
	lightConstants.useSpecularMapDebugFlag = (int)m_specularMap;
	lightConstants.worldEyePosition = m_player->m_position;

	g_theRenderer->SetLightingConstants(lightConstants);

	m_grid->RenderUnlit();
	
	if (m_showModel)
	{
		m_model->m_renderDebug = m_renderDebug;
		m_model->Render();
	}
	else
	{
		for (size_t i = 0; i < m_scenes[m_sceneIdx]->m_entities.size(); i++)
		{
			m_scenes[m_sceneIdx]->m_entities[i]->m_renderDebug = m_renderDebug;
			m_scenes[m_sceneIdx]->m_entities[i]->Render();
		}
	}
	g_theRenderer->EndCamera(m_player->m_playerCamera);
	g_theRenderer->RenderEmissive();
	DebugRenderWorld(m_player->m_playerCamera);

	//ScreenSpace Rendering
	DebugRenderScreen(m_screenCamera);
}

void Game::ShutDown()
{
	for (int i = 0; i < m_scenes.size(); i++)
	{
		delete m_scenes[i];
	}
	delete m_model;
	delete m_player;
}

Scene::~Scene()
{
	for (int i = 0; i < m_entities.size(); i++)
	{
		delete m_entities[i];
	}
}
