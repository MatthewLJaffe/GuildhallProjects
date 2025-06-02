#pragma once
#include "Game/GameCommon.hpp"

enum VisualTestType
{
	VISUAL_TEST_A5,
	VISUAL_TEST_A6,
	RAYCAST_VS_LINESEG2D,
	RAYCAST_VS_AABB2D,
	SHAPES_AND_QUERIES_3D,
	EASING_CURVES_AND_SPLINES,
	PACHINKO_2D,
	DRAWING_SPLINE,
	CONVEX_SCENE_EDITOR,
	NUM_VISUAL_TESTS
};

class VisualTest
{
public:
	VisualTest(VisualTestType myTestType, Game* game);
	virtual ~VisualTest() = default;
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() = 0;
	virtual void InitializeTest() = 0;
	virtual void RandomizeTest() = 0;
	Vec2 RollRandomPositionOnScreen(float excludedBoarder = 0.f) const;
	VisualTestType m_visualTestType;
	Game* m_game = nullptr;
};