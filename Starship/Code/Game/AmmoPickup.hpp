#include "Game/Entity.hpp"

enum AmmoType
{
	AMMO_MISSILE,
	AMMO_LASER
};

class AmmoPickup : public Entity
{
public:
	AmmoPickup(Game* game, Vec2 startPos);
	void Update(float deltaSeconds) override;
	void Render() const override;
protected:
	void InitializeLocalVerts() override;
private:
	AmmoType m_ammoType;
	float m_liveTime = 15.f;
	float m_alphaNormalized = 1.f;
};