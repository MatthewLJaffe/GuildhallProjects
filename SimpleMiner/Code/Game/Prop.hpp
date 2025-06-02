#pragma once
#include "Game/Entity.hpp"

class Texture;

class Prop : public Entity
{
public:
	Prop(Game* game, Vec3 const& startPos);
	void Update(float deltaSeconds) override;
	void Render() const override;
	std::vector<Vertex_PCU> m_vertexes;
	Texture* m_texture = nullptr;
};