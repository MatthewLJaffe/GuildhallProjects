#include "Game/VisualTest.hpp"

VisualTest::VisualTest(VisualTestType myTestType, Game* game)
	: m_visualTestType(myTestType)
	, m_game(game)
{
}

Vec2 VisualTest::RollRandomPositionOnScreen(float excludedBoarder) const
{
	return Vec2( g_randGen->RollRandomFloatInRange(0.f + excludedBoarder, GetScreenWidth() - excludedBoarder), 
		g_randGen->RollRandomFloatInRange(0.f + excludedBoarder, GetScreenHeight() - excludedBoarder) );
}
