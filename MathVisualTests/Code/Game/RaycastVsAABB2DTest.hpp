#pragma once
#include "VisualTest.hpp"
#include "Game/GameCommon.hpp"

constexpr int NUM_AABB2S = 6;

class RaycastVsAABB2DTest : public VisualTest
{
public:
	RaycastVsAABB2DTest(VisualTestType myTestType, Game* game);
	~RaycastVsAABB2DTest() = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void InitializeTest() override;
	void RandomizeTest() override;
	Vec2 m_arrowStartPos;
	Vec2 m_arrowEndPos;
	RaycastResult2D m_closestRaycast;
	int m_lineSegmentHitIdx = -1;
	int m_AABB2HitIdx = -1;
	std::vector<AABB2> m_aabb2s;
	std::vector<Vertex_PCU> m_textVerts;
	float m_startMoveSpeed = 25.f;
	float m_arrowTranslateSpeed = 25.f;
};