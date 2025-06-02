#include "Game/A5VisualTest.hpp"
#include "Game/Game.hpp"

A5VisualTest::A5VisualTest(VisualTestType myTestType, Game* game)
	: VisualTest(myTestType, game)
{
}

void A5VisualTest::InitializeTest()
{
	g_theInput->SetCursorMode(false, false);
	m_point = Vec2(GetScreenWidth() * .5f, GetScreenHeight() * .5f);

	Vec2 aab2Mins = RollRandomPositionOnScreen(30.f);
	Vec2 aab2Maxs = aab2Mins + Vec2(g_randGen->RollRandomFloatInRange(5.f, 40.f), g_randGen->RollRandomFloatInRange(5.f, 40.f));
	m_aabb2 = AABB2(aab2Mins, aab2Maxs);
	
	float oob2Rotation = g_randGen->RollRandomFloatInRange(0.f, 360.f);
	Vec2 oobHalfDimensions = Vec2(g_randGen->RollRandomFloatInRange(2.5f, 20.f), g_randGen->RollRandomFloatInRange(2.5f, 20.f));
	m_obb2 = OBB2(RollRandomPositionOnScreen(30.f), oobHalfDimensions, oob2Rotation);
	
	m_lineSegmentStart = RollRandomPositionOnScreen(20.f);
	m_lineSegmentEnd = m_lineSegmentStart + (g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(5.f, 20.f));

	m_capsuleBoneStart = RollRandomPositionOnScreen(20.f);
	m_capsuleBoneEnd = m_capsuleBoneStart + (g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(5.f, 20.f));
	m_capsuleRadius = g_randGen->RollRandomFloatInRange(3.f, 10.f);

	m_discPos = RollRandomPositionOnScreen(20.f);
	m_discRadius = g_randGen->RollRandomFloatInRange(3.f, 10.f);

	m_infiniteLineSegmentPos = RollRandomPositionOnScreen(20.f);
	m_infiniteLineSegmentDir = g_randGen->RollRandomNormalizedVec2();
	g_bitmapFont->AddVertsForText2D(m_textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .95f), 1.75f, "(Mode F6/F7 for prev/next): Nearest point (2D)", Rgba8(255, 250, 100));
	g_bitmapFont->AddVertsForText2D(m_textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .9f), 1.75f,
		"F8 to randomize; WASD to move point; arrow keys to move point; hold T = slow", Rgba8(100, 255, 200));
}

void A5VisualTest::RandomizeTest()
{
	Vec2 aab2Mins = RollRandomPositionOnScreen(30.f);
	Vec2 aab2Maxs = aab2Mins + Vec2(g_randGen->RollRandomFloatInRange(5.f, 40.f), g_randGen->RollRandomFloatInRange(5.f, 40.f));
	m_aabb2 = AABB2(aab2Mins, aab2Maxs);

	float oob2Rotation = g_randGen->RollRandomFloatInRange(0.f, 360.f);
	Vec2 oobHalfDimensions = Vec2(g_randGen->RollRandomFloatInRange(2.5f, 20.f), g_randGen->RollRandomFloatInRange(2.5f, 20.f));
	m_obb2 = OBB2(RollRandomPositionOnScreen(30.f), oobHalfDimensions, oob2Rotation);

	m_lineSegmentStart = RollRandomPositionOnScreen(20.f);
	m_lineSegmentEnd = m_lineSegmentStart + (g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(5.f, 20.f));

	m_capsuleBoneStart = RollRandomPositionOnScreen(20.f);
	m_capsuleBoneEnd = m_capsuleBoneStart + (g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(5.f, 20.f));
	m_capsuleRadius = g_randGen->RollRandomFloatInRange(3.f, 10.f);

	m_discPos = RollRandomPositionOnScreen(20.f);
	m_discRadius = g_randGen->RollRandomFloatInRange(3.f, 10.f);

	m_infiniteLineSegmentPos = RollRandomPositionOnScreen(20.f);
	m_infiniteLineSegmentDir = g_randGen->RollRandomNormalizedVec2();
}

void A5VisualTest::Update(float deltaSeconds)
{
	Vec2 moveDir(0, 0);
	if (g_theInput->IsKeyDown('W') || g_theInput->IsKeyDown(KEYCODE_UP_ARROW))
	{
		moveDir.y += 1.f;
	}
	if (g_theInput->IsKeyDown('A') || g_theInput->IsKeyDown(KEYCODE_LEFT_ARROW))
	{
		moveDir.x += -1.f;
	}
	if (g_theInput->IsKeyDown('S') || g_theInput->IsKeyDown(KEYCODE_DOWN_ARROW))
	{
		moveDir.y += -1.f;
	}
	if (g_theInput->IsKeyDown('D') || g_theInput->IsKeyDown(KEYCODE_RIGHT_ARROW))
	{
		moveDir.x += 1.f;
	}
	if (moveDir.GetLength() > 0)
	{
		moveDir.SetLength(m_pointMoveSpeed);
	}
	m_point += moveDir * deltaSeconds;
}

void A5VisualTest::Render()
{
	g_theRenderer->BeginCamera(m_game->m_screenCamera);
	Rgba8 pointOutsideColor = Rgba8(50, 80, 150, 255);
	Rgba8 pointInsideColor = Rgba8(100, 150, 255, 255);
	Rgba8 nearestPointColor = Rgba8(255, 160, 0, 255);
	Rgba8 lineToPointColor = Rgba8(255, 255, 255, 30);

	std::vector<Vertex_PCU> testVerts;
	testVerts.reserve(300);

	//is point inside checks

	//AAB2D
	if (IsPointInsideAABB2D(m_point, m_aabb2))
	{
		AddVertsForAABB2D(testVerts, m_aabb2, pointInsideColor);
	}
	else
	{
		AddVertsForAABB2D(testVerts, m_aabb2, pointOutsideColor);
	}
	//OBB2D
	if (IsPointInsideOBB2D(m_point, m_obb2))
	{
		AddVertsForOOB2D(testVerts, m_obb2, pointInsideColor);
	}
	else
	{
		AddVertsForOOB2D(testVerts, m_obb2, pointOutsideColor);
	}
	//capsule
 	if (IsPointInsideCapsule2D(m_point, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius))
	{
		AddVertsForCapsule2D(testVerts, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius, pointInsideColor);
	}
	else
	{
		AddVertsForCapsule2D(testVerts, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius, pointOutsideColor);
	}

	AddVertsForLine2D(testVerts, m_lineSegmentStart, m_lineSegmentEnd, .5f, pointOutsideColor);
	if (IsPointInsideDisc2D(m_point, m_discPos, m_discRadius))
	{
		AddVertsForDisc2D(testVerts, m_discPos, m_discRadius, pointInsideColor);
	}
	else
	{
		AddVertsForDisc2D(testVerts, m_discPos, m_discRadius, pointOutsideColor);
	}
	Vec2 infiniteLineStart = m_infiniteLineSegmentPos - m_infiniteLineSegmentDir * 1000.f;
	Vec2 infiniteLineEnd = m_infiniteLineSegmentPos + m_infiniteLineSegmentDir * 1000.f;
	AddVertsForLine2D(testVerts, infiniteLineStart, infiniteLineEnd, .5f, pointOutsideColor);

	//nearest point checks
	Vec2 nearestDiscPoint =  GetNearestPointOnDisc2D(m_point, m_discPos, m_discRadius);
	Vec2 nearestAABB2Point = GetNearestPointOnAABB2D(m_point, m_aabb2);
	Vec2 nearestLineSegmentPoint = GetNearestPointOnLineSegment2D(m_point, m_lineSegmentStart, m_lineSegmentEnd);
	Vec2 nearestInfiniteLinePoint = GetNearestPointOnInfiniteLine2D(m_point, m_infiniteLineSegmentPos, m_infiniteLineSegmentPos + m_infiniteLineSegmentDir);
	Vec2 nearestCapsulePoint = GetNearestPointOnCapsule2D(m_point, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius);
	Vec2 nearestOBB2Point = GetNearestPointOnOBB2D(m_point, m_obb2);

	AddVertsForLine2D(testVerts, m_point, nearestDiscPoint, .1f, lineToPointColor);;
	AddVertsForLine2D(testVerts, m_point, nearestAABB2Point, .1f, lineToPointColor);;
	AddVertsForLine2D(testVerts, m_point, nearestLineSegmentPoint, .1f, lineToPointColor);;
	AddVertsForLine2D(testVerts, m_point, nearestInfiniteLinePoint, .1f, lineToPointColor);;
	AddVertsForLine2D(testVerts, m_point, nearestCapsulePoint, .1f, lineToPointColor);;
	AddVertsForLine2D(testVerts, m_point, nearestOBB2Point, .1f, lineToPointColor);;

	AddVertsForDisc2D(testVerts, nearestDiscPoint, .5f, nearestPointColor);
	AddVertsForDisc2D(testVerts, nearestAABB2Point, .5f, nearestPointColor);
	AddVertsForDisc2D(testVerts, nearestLineSegmentPoint, .5f, nearestPointColor);
	AddVertsForDisc2D(testVerts, nearestInfiniteLinePoint, .5f, nearestPointColor);
	AddVertsForDisc2D(testVerts, nearestCapsulePoint, .5f, nearestPointColor);
	AddVertsForDisc2D(testVerts, nearestOBB2Point, .5f, nearestPointColor);

	//controllable point
	AddVertsForDisc2D(testVerts, m_point, .25f, Rgba8::WHITE);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(testVerts.size(), testVerts.data());
	g_theRenderer->BindTexture(g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(m_textVerts.size(), m_textVerts.data());
	g_theRenderer->EndCamera(m_game->m_screenCamera);
}

