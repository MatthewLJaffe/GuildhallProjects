#pragma once
#include "VisualTest.hpp"
#include "Game/GameCommon.hpp"

constexpr int NUM_CIRCLES = 10;

struct Circle
{
	Circle(Vec2 pos, float radius);
	Vec2 m_position;
	float m_radius;
};

class A6VisualTest : public VisualTest
{
public:
	A6VisualTest(VisualTestType myTestType, Game* game);
	~A6VisualTest() = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void InitializeTest() override;
	void RandomizeTest() override;
	Vec2 m_arrowStartPos;
	Vec2 m_arrowEndPos;
	RaycastResult2D m_closestRaycast;
	int m_circleHitIdx = -1;
	std::vector<Circle> m_circles;
	float m_startMoveSpeed = 25.f;
	float m_arrowTranslateSpeed = 25.f;
	std::vector<Vertex_PCU> m_textVerts;
};