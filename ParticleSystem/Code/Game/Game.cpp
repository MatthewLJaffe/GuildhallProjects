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
#include "ThirdParty/imgui/imgui.h"
#include "Engine/ParticleSystem/ParticleEmitter.hpp"
#include "Engine/ParticleSystem/Particle.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Engine/ParticleSystem/IndividualParticleEffect.hpp"
#include "Game/TankProjectile.hpp"
#include "Game/Model.hpp"
#include "Game/Scene.hpp"

#include "Game/StartScene.hpp"
#include "Game/FireScene.hpp"
#include "Game/TankScene.hpp"
#include "Game/PerformanceScene.hpp"
#include "Game/BigEffectScene.hpp"
#include "Game/EffectScene.hpp"
#include "Game/ParticleEntity.hpp"


Game::Game(App* app)
	: m_theApp(app)
{}

Game::~Game() {}

void Game::StartUp()
{
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/EarthShatter.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/BlueTankDestroyEffect.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/RedTankDestroyEffect.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/FireV1.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/Head.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/IceLance.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/IceLanceBurst.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/LargeFire.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/MediumFire.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/WildFire.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/Explosion.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/EnergyExplosion.xml");
	g_theParticleSystem->LoadEffectByFileName("Data/Saves/ParticleEffects/LaserBeam.xml");

	m_scenes.push_back(new StartScene());
	m_scenes.push_back(new EffectScene());
	m_scenes.push_back(new FireScene());
	m_scenes.push_back(new TankScene());
	m_scenes.push_back(new PerformanceScene());
	m_scenes.push_back(new BigEffectScene());

	for (int i = 0; i < (int)m_scenes.size(); i++)
	{
		m_scenes[i]->StartUp();
	}

	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");

	m_player = new Player(this, Vec3::ZERO);
	m_allEntities.push_back(m_player);
	//AddTestCube();
	AddGrid();
	//AddGroundPlane();
	font = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont");
	m_screenCamera.SetOrthographicView(Vec2::ZERO, Vec2(1600.f, 800.f));

	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_player->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .1f, 1000.f);
	m_player->m_playerCamera.m_mode = Camera::eMode_Perspective;
	m_player->m_playerCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	//m_player->m_position = Vec3(-230.f, 0.f, 35.f);
	m_player->m_position = Vec3(-15.f, 0.f, 5.f);

	m_player->m_orientationDegrees.m_yaw = 0.f;
	m_player->m_orientationDegrees.m_pitch = 10.f;
	m_player->m_playerCamera.m_position = m_player->m_position;
	m_player->m_playerCamera.m_orientation = m_player->m_orientationDegrees;

	g_theParticleSystem->m_config.m_playerCamera = &m_player->m_playerCamera;
	m_skyboxTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/cubemapNoSun.png");
}

Vec3 Game::reconstructPos(Vec2 const& uv, float z, Mat44 const& inverseViewProj)
{
	float x = uv.x * 2 - 1;
	float y = (1 - uv.y) * 2 - 1; //[1,0] to [-1,1]

	//ndc space position
	Vec4 position_s = Vec4(x, y, z, 1);
	float wValue = computeLinearDepth(z);
	Vec4 unWDividedPos = position_s * wValue;
	Vec4 newPos = inverseViewProj.TransformHomogeneous3D(unWDividedPos);
	return newPos.GetXYZ();
}

float Game::computeLinearDepth(float z)
{
	float c_nearPlane = m_player->m_playerCamera.GetPerspectiveNear();
	float c_farPlane = m_player->m_playerCamera.GetPerspectiveFar();

	float linZ = (c_nearPlane * c_farPlane) / (c_farPlane + z * c_nearPlane - z * c_farPlane);
	return linZ;
}

void Game::AddTestCube()
{
	Prop* testWall = new Prop(g_theGame, Vec3(7.f, 0.f, 0.f));
	AABB3 testWallBounds = AABB3(-1.f, -5.f, 0.f, 1.f, 5.f, 5.f);
	testWallBounds.m_mins += Vec3(7.f, 0.f, 0.f);
	testWallBounds.m_maxs += Vec3(7.f, 0.f, 0.f);
	AddVertsForAABB3D(testWall->m_vertexes, testWallBounds);
	m_allEntities.push_back(testWall);
	//m_testWallCollider = g_theParticleSystem->AddParticlePhysicsAABB3(Vec3(7.f, 0.f, 0.f) + Vec3(-2.5f, -5.f, 0.f), Vec3(7.f, 0.f, 0.f) + Vec3(2.5f, 5.f, 5.f));
}

void Game::AddGrid()
{
	m_grid = new Prop(this, Vec3::ZERO);
	std::vector<Vertex_PCU> gridVerts;
	int gridLength = 40;

	for (int x = -gridLength; x < gridLength; x++)
	{
		Rgba8 color(200, 200, 200);
		if (x <= 0)
		{
			color.a = (unsigned char)Lerp(255.f, 0.f, (float)x / -(float)gridLength);
		}
		else
		{
			color.a = (unsigned char)Lerp(255.f, 0.f, (float)x / (float)gridLength);
		}
		AddVertsForLine3D(gridVerts, Vec3((float)x, -(float)gridLength, 0.f), Vec3((float)x, 0.f, 0.f), .025f, Rgba8(200, 200, 200,0), color, AABB2::ZERO_TO_ONE, 4);
		AddVertsForLine3D(gridVerts, Vec3((float)x, (float)gridLength, 0.f), Vec3((float)x, 0.f, 0.f), .025f, Rgba8(200, 200, 200, 0), color, AABB2::ZERO_TO_ONE, 4);
	}

	for (int y = -gridLength; y < gridLength; y++)
	{
		Rgba8 color(200, 200, 200);

		if (y <= 0)
		{
			color.a = (unsigned char)Lerp(255.f, 0.f, (float)y / -(float)gridLength);
		}
		else
		{
			color.a = (unsigned char)Lerp(255.f, 0.f, (float)y / (float)gridLength);
		}
		AddVertsForLine3D(gridVerts, Vec3(-(float)gridLength, (float)y, 0.f), Vec3(0.f, (float)y, 0.f), .025f, Rgba8(200, 200, 200, 0), color, AABB2::ZERO_TO_ONE, 4);
		AddVertsForLine3D(gridVerts, Vec3(0.f, (float)y, 0.f), Vec3((float)gridLength, (float)y, 0.f), .025f, color, Rgba8(200, 200, 200, 0), AABB2::ZERO_TO_ONE, 4);
	}
	
	m_grid->m_vertexes = gridVerts;
	m_allEntities.push_back(m_grid);
}

void Game::AddGroundPlane()
{
	m_groundPlane = new Prop(this, Vec3::ZERO);
	std::vector<Vertex_PCU> groundVerts;
	AddVertsForQuad3D(groundVerts, Vec3(-500.f, 500.f, 0.f), Vec3(-500.f, -500.f, 0.f), Vec3(500.f, -500.f, 0.f), Vec3(500.f, 500.f, 0.f));
	m_groundPlane->m_vertexes = groundVerts;
	m_groundPlane->m_color = Rgba8(100, 100, 100);
	m_allEntities.push_back(m_groundPlane);
}

void Game::ToggleDebugDrawVectorField()
{
	if (m_debugArrowsGPU != nullptr)
	{
		delete m_debugArrowsGPU;
		m_debugArrowsGPU = nullptr;
		return;
	}

	Vec3 startPos = m_player->m_position;
	int fieldLength = 20;

	std::vector<Vertex_PCU> fieldVerts;
	int stepSize = 2;
	fieldVerts.reserve((fieldLength/stepSize) * (fieldLength/stepSize) * (fieldLength/stepSize) * 36);
	for (int z = -fieldLength/2; z < fieldLength/2; z+=stepSize)
	{
		for (int y = -fieldLength/2; y < fieldLength/2; y+=stepSize)
		{
			for (int x = -fieldLength/2; x < fieldLength/2; x+=stepSize)
			{
				IntVec3 coords = IntVec3((int)startPos.x + x, (int)startPos.y + y, (int)startPos.z + z);
				Vec3 direction;
				direction.x = Compute3dPerlinNoise((float)coords.x, (float)coords.y, (float)coords.z, 40.f, 7, .5f, 2.f, true, 0);
				direction.y = Compute3dPerlinNoise((float)coords.x, (float)coords.y, (float)coords.z, 40.f, 7, .5f, 2.f, true, 1);
				direction.z = Compute3dPerlinNoise((float)coords.x, (float)coords.y, (float)coords.z, 40.f, 7, .5f, 2.f, true, 2);

				direction = direction.GetNormalized();
				float colorAsFloats[4];
				colorAsFloats[0] = direction.x * .5f + .5f;
				colorAsFloats[1] = direction.y * .5f + .5f;
				colorAsFloats[2] = direction.z * .5f + .5f;
				colorAsFloats[3] = 1.f;

				Rgba8 color;
				color.SetFromFloats(colorAsFloats);
				AABB3 point(coords.GetVec3() - Vec3(.02f, .02f, .02f), coords.GetVec3() + Vec3(.02f, .02f, .02f));
				AddVertsForAABB3D(fieldVerts, point);
				AddVertsForBoxLine3D(fieldVerts, coords.GetVec3(), coords.GetVec3() + direction * .5f, .01f, color);
			}
		}
	}
	m_debugArrowsGPU = new GPUMesh(fieldVerts);
}


void Game::CheckForDebugCommands()
{
	if (g_theInput->WasKeyJustPressed('F'))
	{
		ToggleDebugDrawVectorField();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_TAB))
	{
		g_theParticleSystem->ToggleParticleEditor();
		m_controllingPlayer = !m_controllingPlayer;
	}
}

void Game::DeleteDeadEntities()
{
	for (int i = 0; i < (int)m_allEntities.size(); i++)
	{
		if (m_allEntities[i] != nullptr && m_allEntities[i]->m_isAlive == false)
		{
			delete m_allEntities[i];
			m_allEntities[i] = nullptr;
		}
	}
}

void Game::ShootBullet()
{

	ParticleEffect* iceLanceEffect = g_theParticleSystem->AddParticleEffectByFileName("Data/Saves/ParticleEffects/IceLance.xml", Vec3::ZERO, EulerAngles(), true);
	ParticlePhysicsObject* physicsObject = g_theParticleSystem->AddParticlePhysicsObject(Vec3::ZERO, 5.f, 500.f);
	Vec3 bulletStartPos = m_player->m_position - m_player->m_orientationDegrees.GetKUp() - m_player->m_orientationDegrees.GetJLeft();

	ParticleEntity* bullet = new ParticleEntity(this, bulletStartPos, m_player->m_orientationDegrees, iceLanceEffect, physicsObject, 5.f);

	bullet->m_velocity = 25.f * ((m_player->m_position + m_player->m_orientationDegrees.GetIFwd() * 50.f) - bulletStartPos).GetNormalized();
	bullet->m_liveTime = 5.f;
	bullet->m_isAlive = true;
	bullet->m_hasLifetime = true;
	bullet->m_orientationDegrees = m_player->m_orientationDegrees;
	bullet->m_orientationDegrees.m_pitch -= 2.5f;
	bullet->m_orientationDegrees.m_yaw += 2.5f;
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == nullptr)
		{
			m_allEntities[i] = bullet;
			return;
		}
	}

	m_allEntities.push_back(bullet);
}

void Game::ShootBigBullet()
{
	Prop* bullet = new Prop(this, m_player->m_position + m_player->m_orientationDegrees.GetIFwd() * .5f, true);
	bullet->m_physicsObject->SetRadius(3.f);
	bullet->m_physicsObject->SetForceMagnitude(1500.f);
	bullet->m_velocity = 50.f * m_player->m_orientationDegrees.GetIFwd();
	bullet->m_liveTime = 5.f;
	bullet->m_isAlive = true;
	bullet->m_hasLifetime = true;
	bullet->m_physicsBounds = AABB3(Vec3(-.25f, -.25f, -.25f), Vec3(.25f, .25f, .25f));
	AddVertsForSphere3D(bullet->m_vertexes, Vec3::ZERO, 1.5f);
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == nullptr)
		{
			m_allEntities[i] = bullet;
			return;
		}
	}

	m_allEntities.push_back(bullet);
}

void Game::SwitchScenes(bool forward)
{
	g_theParticleSystem->KillAllEmitters();
	m_scenes[m_sceneIndex]->SwitchOff();
	if (forward)
	{
		m_sceneIndex++;
		if (m_sceneIndex > (int)m_scenes.size() - 1)
		{
			m_sceneIndex = 0;
		}
	}
	else
	{
		m_sceneIndex--;
		if (m_sceneIndex < 0)
		{
			m_sceneIndex = (int)m_scenes.size() - 1;
		}
	}
	m_scenes[m_sceneIndex]->SwitchOn();
}

void Game::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_SQUARE_BRACKET))
	{
		SwitchScenes(false);
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_RIGHT_SQUARE_BRACKET))
	{
		SwitchScenes(true);
	}
	
	m_scenes[m_sceneIndex]->Update(deltaSeconds);
	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == nullptr)
		{
			continue;
		}
		m_allEntities[i]->Update(deltaSeconds);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		ShootBullet();
	}
	DeleteDeadEntities();
	g_theParticleSystem->Update(deltaSeconds);
	DebugAddMessage(Stringf("%.2f MS", g_theApp->m_lastFrameTime * 1000.0), 20.f, 0.f);
	DebugAddMessage("Controls:", 20.f, 0.f);
	DebugAddMessage("Tab toggle editor [/] Switch Scenes", 20.f, 0.f);
	CheckForDebugCommands();
}

void Game::Render() 
{
	g_theRenderer->SetPrimaryRenderTargetType(PrimaryRenderTargetType::HDR);
	//World Space Rendering
	g_theRenderer->BeginCamera(m_player->m_playerCamera);

	//skybox
	std::vector<Vertex_PCU> skyboxVerts;
	AddVertsForCubeMapSkyBox(skyboxVerts, m_player->m_position, 20.f);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(m_skyboxTexture);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray(skyboxVerts.size(), skyboxVerts.data());

	LightConstants lightConstants;
	float sunIntensity = .5f;
	EulerAngles sunOrientaiton = EulerAngles(120.f, 45.f, 0.f);

	lightConstants.AmbientIntensity = 1.f - sunIntensity;
	lightConstants.SunDirection = sunOrientaiton.GetIFwd();
	lightConstants.SunIntensity = sunIntensity;
	lightConstants.worldEyePosition = m_player->m_playerCamera.m_position;
	g_theRenderer->SetLightingConstants(lightConstants);

	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == nullptr)
		{
			continue;
		}
		m_allEntities[i]->Render();
	}
	m_scenes[m_sceneIndex]->Render();
	g_theParticleSystem->Render();
	if (m_debugArrowsGPU)
	{
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->BindShader(nullptr);
		m_debugArrowsGPU->Render();
	}
	g_theRenderer->RenderEmissive();
	g_theRenderer->CompositeHDR();
	g_theRenderer->EndCamera(m_player->m_playerCamera);
	DebugRenderWorld(m_player->m_playerCamera);

	//ScreenSpace Rendering
	DebugRenderScreen(m_screenCamera);
}

void Game::EndFrame()
{
}

void Game::ShutDown()
{
	if (m_debugArrowsGPU != nullptr)
	{
		delete m_debugArrowsGPU;
		m_debugArrowsGPU = nullptr;
	}
	for (int i = 0; i < m_scenes.size(); i++)
	{
		delete m_scenes[i];
		m_scenes[i] = nullptr;
	}
}

Scene* Game::GetCurrentScene()
{
	return m_scenes[m_sceneIndex];
}


