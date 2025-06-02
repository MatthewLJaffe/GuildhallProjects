#pragma once
#include "VisualTest.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"


struct MobileDisc
{
	MobileDisc(Vec2 pos, Vec2 veloctiy, float radius);
	Vec2 m_position;
	Vec2 m_velocity;
	Vec2 m_dir;
	float m_radius;
	float m_elasticity = .9f;
	Rgba8 m_color;
};

class Bumper
{
public:
	Bumper();
	virtual ~Bumper() = default;
	void Render();
	bool BounceMobileDisc(MobileDisc& discToBounce);
	virtual Vec2 GetNearestPointOnBumper(Vec2 const& discPos) const = 0;
	float m_elasticity = .8f;
	std::vector<Vertex_PCU> m_bumperVerts;
	Vec2 m_boundingCirclePos;
	Rgba8 m_color;
	float m_boundingCircleRadius;
};

class CapsuleBumper : public Bumper
{
public:
	CapsuleBumper(Vec2 boneStartPos, Vec2 boneEndPos, float boneRadius);
	Vec2 GetNearestPointOnBumper(Vec2 const& discPos) const override;
	Vec2 m_boneStartPos;
	Vec2 m_boneEndPos;
	float m_boneRadius = 0.f;
};

class DiscBumper : public Bumper
{
public:
	DiscBumper(Vec2 discPos, float discRadius);
	Vec2 GetNearestPointOnBumper(Vec2 const& discPos) const override;
	Vec2 m_discPos;
	float m_discRadius = 0.f;
};

class OBBBumper : public Bumper
{
public:
	OBBBumper(OBB2 obb);
	OBB2 m_box;
	Vec2 GetNearestPointOnBumper(Vec2 const& discPos) const override;
};

class Pachinkio2DVisualTest : public VisualTest
{
public:
	Pachinkio2DVisualTest(VisualTestType myTestType, Game* game);
	~Pachinkio2DVisualTest();
	void Update(float deltaSeconds) override;
	void UpdatePhysics();
	void UpdateArrow(float deltaSeconds);
	void Render() override;
	void InitializeTest() override;
	void RandomizeTest() override;
	Vec2 m_arrowStartPos = Vec2(50.f, 25.f);
	Vec2 m_arrowEndPos = Vec2(150.f, 75.f);
	std::vector<Bumper*> m_bumpers;
	std::vector<MobileDisc> m_mobileDiscs;
	float m_startMoveSpeed = 25.f;
	float m_arrowTranslateSpeed = 25.f;
	std::vector<Vertex_PCU> m_textVerts;
	bool m_bottomWarp = false;
	float m_physicsTimeDebt = 0.f;
};

constexpr int NUM_DISC_BUMPERS = 10;
constexpr int NUM_CAPSULE_BUMPERS = 10;
constexpr int NUM_OBB_BUMPERS = 10;
constexpr float MOBILE_DISC_MIN_SIZE = 1.f;
constexpr float MOBILE_DISC_MAX_SIZE = 2.f;
constexpr float GRAVITY = 75.f;