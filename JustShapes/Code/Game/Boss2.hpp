#pragma once
#include "Game/Entity.hpp"

class Boss2 : public Entity 
{
public:
	Boss2(GameState* gameState, EntityType entityType, Vec2 const& startPos);
	void Update(float deltaSeconds) override;
	void Render() override;
	void Spawn();
	void Teleport(Vec2 teleportPos, bool hideAfterTeleport = false);
	void PlayDevilAnimation();
	bool OverlapsPlayer(Player* player) override;
	Timer m_hideTimer;
	bool m_hidden = false;
};