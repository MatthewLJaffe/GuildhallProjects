#include "Game/StartScene.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"
#include "Game/Player.hpp"

void StartScene::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void StartScene::StartUp()
{
}

void StartScene::SwitchOn()
{
	g_theParticleSystem->ToggleParticleEditor(true);
	g_theGame->m_controllingPlayer = false;
	g_theGame->m_player->m_position = Vec3(-15.f, 0.f, 5.f);
	g_theGame->m_player->m_orientationDegrees = EulerAngles(0.f, 10.f , 0.f);

	g_theGame->m_player->m_playerCamera.m_position = g_theGame->m_player->m_position;
	g_theGame->m_player->m_playerCamera.m_orientation = g_theGame->m_player->m_orientationDegrees;


}

void StartScene::SwitchOff()
{
	Scene::SwitchOff();
	g_theGame->m_controllingPlayer = true;
	g_theParticleSystem->ToggleParticleEditor(false);
}
