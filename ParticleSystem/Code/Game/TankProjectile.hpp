#pragma once
#include "Game/Entity.hpp"
#include "Engine/ParticleSystem/ParticleSystem.hpp"

class TankProjectile : public Entity
{
public:
	TankProjectile(Game* game, Vec3 const& startPos, bool isBlueTeam);
	~TankProjectile();
	void Update(float deltaSeconds) override;
	void Render() const override;

	std::vector<Vertex_PCU> m_vertexes;
	Texture* m_texture = nullptr;
	Entity* m_tankToDestroy;
	bool m_isPhysicsObject = false;
	Vec3 m_startPos;
	float m_travelDistance = 5.f;
	bool m_isBlueTeam = true;
	ParticlePhysicsObject* m_physicsObject = nullptr;
	ParticleEffect* m_particleEffect = nullptr;
};