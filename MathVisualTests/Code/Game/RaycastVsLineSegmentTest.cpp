#include "RaycastVsLineSegmentTest.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/Game.hpp"

RaycastVsLineSegmentTest::RaycastVsLineSegmentTest(VisualTestType myTestType, Game* game)
	: VisualTest(myTestType, game)
{
}

void RaycastVsLineSegmentTest::InitializeTest()
{
	g_theInput->SetCursorMode(false, false);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	m_arrowStartPos = Vec2(GetScreenWidth() * .5f, GetScreenHeight() * .5f) - Vec2(1.f, .25f) * 30.f;
	m_arrowEndPos = Vec2(GetScreenWidth() * .5f, GetScreenHeight() * .5f) + Vec2(1.f, .25f) * 30.f;
	m_lineSegments.clear();

	for (int i = 0; i < NUM_LINESEGMENTS; i++)
	{
		LineSegment lineSegment;
		lineSegment.m_startPoint = RollRandomPositionOnScreen(20.f);
		lineSegment.m_endPoint = lineSegment.m_startPoint + (g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(5.f, 40.f));
		m_lineSegments.push_back(lineSegment);
	}

	g_bitmapFont->AddVertsForText2D(m_textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .95f), 1.75f, "(Mode F6/F7 for prev/next): 2D Ray vs. Segments", Rgba8(255, 250, 100));
	g_bitmapFont->AddVertsForText2D(m_textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .9f), 1.75f,
		"F8 to randomize; LMB/RMB set ray start/end; WASD move start, IJKL move end, arrows move ray, hold T = slow", Rgba8(100, 255, 200));
}

void RaycastVsLineSegmentTest::RandomizeTest()
{
	m_lineSegments.clear();

	for (int i = 0; i < NUM_LINESEGMENTS; i++)
	{
		LineSegment lineSegment;
		lineSegment.m_startPoint = RollRandomPositionOnScreen(20.f);
		lineSegment.m_endPoint = lineSegment.m_startPoint + (g_randGen->RollRandomNormalizedVec2() * g_randGen->RollRandomFloatInRange(5.f, 20.f));
		m_lineSegments.push_back(lineSegment);
	}
}

void RaycastVsLineSegmentTest::Update(float deltaSeconds)
{
	//move arrow start
	Vec2 moveDir(0, 0);
	if (g_theInput->IsKeyDown('W'))
	{
		moveDir.y += 1.f;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveDir.x += -1.f;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveDir.y += -1.f;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveDir.x += 1.f;
	}
	if (moveDir != Vec2::ZERO)
	{
		moveDir.Normalize();
		m_arrowStartPos += moveDir * m_startMoveSpeed * deltaSeconds;
	}

	moveDir = Vec2::ZERO;
	if (g_theInput->IsKeyDown('I'))
	{
		moveDir.y += 1.f;
	}
	if (g_theInput->IsKeyDown('J'))
	{
		moveDir.x += -1.f;
	}
	if (g_theInput->IsKeyDown('K'))
	{
		moveDir.y += -1.f;
	}
	if (g_theInput->IsKeyDown('L'))
	{
		moveDir.x += 1.f;
	}
	if (moveDir != Vec2::ZERO)
	{
		moveDir.Normalize();
		m_arrowEndPos += moveDir * m_startMoveSpeed * deltaSeconds;
	}

	moveDir = Vec2::ZERO;
	if (g_theInput->IsKeyDown(KEYCODE_UP_ARROW))
	{
		moveDir.y += 1.f;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_ARROW))
	{
		moveDir.x += -1.f;
	}
	if (g_theInput->IsKeyDown(KEYCODE_DOWN_ARROW))
	{
		moveDir.y += -1.f;
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_ARROW))
	{
		moveDir.x += 1.f;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 normalizedMousePos = g_theWindow->GetNormalizedCursorPos();
		m_arrowStartPos = Vec2(normalizedMousePos.x * GetScreenWidth(), normalizedMousePos.y * GetScreenHeight());
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 normalizedMousePos = g_theWindow->GetNormalizedCursorPos();
		m_arrowEndPos = Vec2(normalizedMousePos.x * GetScreenWidth(), normalizedMousePos.y * GetScreenHeight());
	}

	moveDir.Normalize();
	Vec2 displacment = moveDir * m_arrowTranslateSpeed * deltaSeconds;
	m_arrowStartPos += displacment;
	m_arrowEndPos += displacment;
	m_closestRaycast = RaycastResult2D();
	Vec2 arrowFwdNormal = (m_arrowEndPos - m_arrowStartPos).GetNormalized();
	float raycastDistance = GetDistance2D(m_arrowStartPos, m_arrowEndPos);
	
	
	m_lineSegmentHitIdx = -1;
	for (int i = 0; i < NUM_LINESEGMENTS; i++)
	{
		LineSegment const& currSeg = m_lineSegments[i];
		RaycastResult2D currCast = RaycastVsLineSegment2D(m_arrowStartPos, arrowFwdNormal, raycastDistance, currSeg.m_startPoint, currSeg.m_endPoint);
		if (currCast.m_didImpact && (currCast.m_impactDist < m_closestRaycast.m_impactDist || !m_closestRaycast.m_didImpact))
		{
			m_closestRaycast = currCast;
			m_lineSegmentHitIdx = i;
		}
	}
}

void RaycastVsLineSegmentTest::Render()
{
	g_theRenderer->BeginCamera(m_game->m_screenCamera);
	Rgba8 missColor = Rgba8(50, 80, 150, 255);
	Rgba8 hitColor = Rgba8(100, 150, 255, 255);
	float lineSegmentWidth = .15f;
	std::vector<Vertex_PCU> verts;
	verts.reserve(1000);
	for (size_t i = 0; i < m_lineSegments.size(); i++)
	{
		AddVertsForLine2D(verts, m_lineSegments[i].m_startPoint ,m_lineSegments[i].m_endPoint, lineSegmentWidth, missColor);
	}


	if (m_closestRaycast.m_didImpact)
	{
		//hit line segment
		if (m_lineSegmentHitIdx != -1)
		{
			AddVertsForLine2D(verts, m_lineSegments[m_lineSegmentHitIdx].m_startPoint, m_lineSegments[m_lineSegmentHitIdx].m_endPoint, lineSegmentWidth, hitColor);
		}

		Vec2 normalStart = m_closestRaycast.m_impactPos;
		Vec2 normalEnd = m_closestRaycast.m_impactPos + m_closestRaycast.m_impactNormal * 4.f;
		AddVertsForArrow2D(verts, m_arrowStartPos, m_arrowEndPos, 1.f, .1f, Rgba8(127, 127, 127));
		AddVertsForArrow2D(verts, m_arrowStartPos, m_closestRaycast.m_impactPos, 1.f, .1f, Rgba8::RED);
		AddVertsForArrow2D(verts, normalStart, normalEnd, 1.f, .1f, Rgba8::YELLOW);
		AddVertsForDisc2D(verts, m_closestRaycast.m_impactPos, .3f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(verts, m_arrowStartPos, m_arrowEndPos, 1.f, .1f, Rgba8::GREEN);
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(verts.size(), verts.data());
	g_theRenderer->BindTexture(g_bitmapFont->GetTexture());
	g_theRenderer->DrawVertexArray(m_textVerts.size(), m_textVerts.data());
	g_theRenderer->EndCamera(m_game->m_screenCamera);
}
