#pragma once
#include "VisualTest.hpp"
#include "Game/GameCommon.hpp"

struct LineSegment
{
	Vec2 m_startPoint;
	Vec2 m_endPoint;
};

constexpr int NUM_LINESEGMENTS = 10;

class RaycastVsLineSegmentTest : public VisualTest
{
public:
	RaycastVsLineSegmentTest(VisualTestType myTestType, Game* game);
	~RaycastVsLineSegmentTest() = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void InitializeTest() override;
	void RandomizeTest() override;
	Vec2 m_arrowStartPos;
	Vec2 m_arrowEndPos;
	RaycastResult2D m_closestRaycast;
	int m_lineSegmentHitIdx = -1;
	std::vector<LineSegment> m_lineSegments;
	std::vector<Vertex_PCU> m_textVerts;
	float m_startMoveSpeed = 25.f;
	float m_arrowTranslateSpeed = 25.f;
};