#include "EditorScene.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Game/Prop.hpp"
#include "Engine/Renderer/Window.hpp"

void EditorScene::StartUp()
{
	if (m_startedUp)
	{
		return;
	}
	g_theGame->m_editorPlayer = new Player(Vec3::ZERO);
	m_allEntities.push_back(g_theGame->m_editorPlayer);
	Prop* grid = new Prop(Vec3::ZERO);
	grid->CreateGrid();
	m_allEntities.push_back(grid);

	g_theGame->m_editorPlayer->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .1f, 1000.f);
	g_theGame->m_editorPlayer->m_playerCamera.m_mode = Camera::eMode_Perspective;
	g_theGame->m_editorPlayer->m_playerCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	g_theGame->m_editorPlayer->m_playerCamera.m_useWorldTransform = true;
	g_theGame->m_editorPlayer->SetLocalPosition(Vec3(-15.f, 0.f, 5.f));


	g_theGame->m_editorPlayer->m_playerCamera.SetTransform(g_theGame->m_editorPlayer->GetWorldTransform());
	return;
}

void EditorScene::Update(float deltaSeconds)
{
	DeleteDeadEntities();
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == nullptr)
		{
			continue;
		}
		m_allEntities[i]->Update(deltaSeconds);
	}
}

void EditorScene::Render() const
{
	Camera* playerCamera = g_theGame->GetPlayerCamera();
	g_theRenderer->SetPrimaryRenderTargetType(PrimaryRenderTargetType::HDR);
	//World Space Rendering
	g_theRenderer->BeginCamera(*playerCamera);

	LightConstants lightConstants;
	float sunIntensity = .3f;
	lightConstants.AmbientIntensity = 1.f - sunIntensity;
	lightConstants.SunDirection = g_theGame->m_sunOrientaiton.GetIFwd();
	lightConstants.SunIntensity = sunIntensity;
	lightConstants.worldEyePosition = playerCamera->m_position;
	g_theRenderer->SetLightingConstants(lightConstants);

	for (size_t i = 0; i < m_allEntities.size(); i++)
	{
		if (m_allEntities[i] == nullptr)
		{
			continue;
		}
		m_allEntities[i]->Render();
	}
	g_theParticleSystem->Render();
	g_theRenderer->RenderEmissive();
	g_theRenderer->CompositeHDR();
	g_theRenderer->EndCamera(*playerCamera);
}

void EditorScene::DeleteDeadEntities()
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

void EditorScene::ShutDown()
{
	for (int i = 0; i < (int)m_allEntities.size(); i++)
	{
		delete m_allEntities[i];
		m_allEntities[i] = nullptr;
	}
	g_theParticleSystem->KillAllEmitters();
}
