#pragma once
#include "Game/Entity.hpp"

class MissileLauncher;
class LaserBeam;

class PlayerShip : public Entity
{
public:
	PlayerShip(Game* game, Vec2 startPos);
	~PlayerShip() override;
	void Update(float deltaSeconds) override;
	void HandlePlayerControlsController(float deltaSeconds);
	void HandlePlayerControlsKeyboard(float deltaSeconds);
	void MovePlayerShip(float deltaSeconds);
	void Render() const override;
	void RenderUI() const;
	void Die() override;
	void Respawn();
	void RenderDebug() const override;
	void AddMissileAmmo(int ammoToAdd);
	void AddLaserAmmo(float ammoToAdd);
	LaserBeam* m_laserBeam;
protected:
	void InitializeLocalVerts() override;
private:
	void RenderShip() const;
	void RenderTail() const;
	Vec3 GetTailPos() const;
	void FireBullet();
	SoundPlaybackID m_laserPlayback;
	MissileLauncher* m_missileLauncher;
	Vertex_PCU m_localShipVerts[NUM_SHIP_VERTS];
	Vertex_PCU m_localTailVerts[NUM_TAIL_VERTS];
	float m_thrustFraction = 0.f;
	int m_extraLives = 3;
	float m_invincibleTime = 0.f;
	float m_thrustLength = 0.f;
};