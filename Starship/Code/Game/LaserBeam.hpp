#include "Game/GameCommon.hpp"
class Game;
class Entity;

class LaserBeam
{
public:
	LaserBeam(Game* game);
	void Update(float deltaSeconds);
	void Render() const;
	bool IsInLaser(Entity* entity);
	void DrawLaserBeam(Vec2 startPos, Vec2 endPos, float thickness, Rgba8 color) const;
	bool m_firing = false;
	float m_damagePerSecond = 20.f;
	float m_ammo = 5.f;
private:
	float m_ammoPerSecond = 1.f;
	float m_thickness = 5.f;
	float m_length = 200.f;
	Game* m_game = nullptr;
	Vec2 m_startPos;
	Vec2 m_endPos;
	Rgba8 m_color;
};