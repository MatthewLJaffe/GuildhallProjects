#include "Boss2.hpp"
#include "Game/GameCommon.hpp"

Boss2::Boss2(GameState* gameState, EntityType entityType, Vec2 const& startPos)
	: Entity(gameState, entityType, startPos)
{
	m_spriteBounds = AABB2(Vec2::ZERO, Vec2(16.f, 16.f));
	//SpriteDefinition const& boss2StartSprite = g_theRenderer->CreateOrGetSpriteSheetFromFile("Data/Images/Boss2Sheet.png", IntVec2(14, 14))->GetSpriteDef(0);
	//m_texture = boss2StartSprite.GetTexture();
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/SmallRing.png");
	//m_uvs = boss2StartSprite.GetUVs();
	m_uvs = AABB2::ZERO_TO_ONE;
	m_color = Rgba8(236, 1, 106, 255);
	m_isHazard = true;
	m_sortOrder = 3;
	m_radius = 8.f;
	m_hideTimer = Timer(1.4f, g_theApp->GetGameClock());
}

void Boss2::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	UpdateAnimation();
	UpdateChildren(deltaSeconds);

	if (!m_hideTimer.IsStopped() && m_hideTimer.HasPeriodElapsed())
	{
		m_hideTimer.Stop();
		m_hidden = true;
	}
}

void Boss2::Render()
{
	if (m_hidden)
	{
		return;
	}
	Entity::Render();
}

void Boss2::Spawn()
{
	m_spriteBounds = AABB2(Vec2::ZERO, Vec2(48.f, 48.f));
	m_radius = 24.f;
	PlayAnimation("Boss2Spawn");
	AddChildEntity(new Entity(m_gameState, EntityType::BOSS_2, Vec2(0, 6), EntityConfig::GetEntityConfigByName("Boss2Hair")));
	AddChildEntity(new Entity(m_gameState, EntityType::BOSS_2, Vec2(0, 0), EntityConfig::GetEntityConfigByName("Boss2Face")));
}

void Boss2::Teleport(Vec2 teleportPos, bool hideAfterTeleport)
{
	m_hidden = false;
	m_position = teleportPos;
	if (hideAfterTeleport)
	{
		m_hideTimer.Start();
		PlayAnimation("TeleportInAndOut");
	}
	else
	{
		PlayAnimation("TeleportIn");
	}

}

void Boss2::PlayDevilAnimation()
{
	m_children[1]->PlayAnimation("DevilFace");
}

bool Boss2::OverlapsPlayer(Player* player)
{
	if (m_hidden)
	{
		return false;
	}
	return Entity::OverlapsPlayer(player);
}
