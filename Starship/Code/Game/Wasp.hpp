#pragma once
#include "Entity.hpp"

class Wasp : public Entity
{
public:
	Wasp(Game* game, const Vec2& pos);
	void Update(float deltaSeconds) override;
	void Render() const override;
	virtual void Die() override;
	Vec2 m_acceleration;
protected:
	void InitializeLocalVerts() override;
};