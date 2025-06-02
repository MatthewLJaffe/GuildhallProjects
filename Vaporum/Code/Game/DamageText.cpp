#include "Game/DamageText.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/Player.hpp"

DamageText::DamageText(Game* game, Vec3 const& startPos, int damage)
	: Entity(game, startPos)
{
	g_theGame->m_bitmapFont->AddVertsForText3DAtOriginXForward(m_debugVertexes, .5f, Stringf("%d", damage), Rgba8::WHITE, .5f);
	m_startPos = startPos;
	m_currentLiveTime = 0.f;
}

void DamageText::Update(float deltaSeconds)
{
	m_position = Vec3::Lerp(m_startPos, m_startPos + m_endOffset, m_currentLiveTime / m_liveTime);
	m_color = LerpColor(m_startColor, m_endColor, SmoothStart3(m_currentLiveTime / m_liveTime));
	m_currentLiveTime += deltaSeconds;

	if (m_currentLiveTime > m_liveTime)
	{
		m_isAlive = false;
	}
}

void DamageText::Render() const
{
	if (!m_isAlive)
	{
		return;
	}

	Mat44 targetMatrix = g_theGame->m_player->m_playerCamera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	targetMatrix.AppendZRotation(90.f);
	targetMatrix.AppendTranslation3D(g_theGame->m_player->m_playerCamera.m_position);

	Mat44 billboardMatrix = GetBillboardMatrix(BillboardType::FULL_OPPOSING, targetMatrix, m_position);
	g_theRenderer->SetModelConstants(billboardMatrix, m_color);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(g_theGame->m_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(m_debugVertexes.size(), m_debugVertexes.data());
}