#pragma once
#include "Game/Entity.hpp"

class Beetle : public Entity
{
public:
	Beetle(Game* game, const Vec2& pos);
	void Update(float deltaSeconds) override;
	void Render() const override;
	virtual void Die() override;
protected:
	void InitializeLocalVerts() override;
};