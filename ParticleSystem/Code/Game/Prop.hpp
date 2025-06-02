#pragma once
#include "Game/Entity.hpp"
#include "Engine/ParticleSystem/ParticlePhysicsObject.hpp"

class Texture;

class Prop : public Entity
{
public:
	Prop(Game* game, Vec3 const& startPos, bool isPhysicsObject = false);
	~Prop();
	void Update(float deltaSeconds) override;
	void Render() const override;
	std::vector<Vertex_PCU> m_vertexes;
	Texture* m_texture = nullptr;
	bool m_isPhysicsObject = false;
	ParticlePhysicsObject* m_physicsObject = nullptr;
};