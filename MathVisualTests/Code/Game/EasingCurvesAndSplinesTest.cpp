#include "Game/EasingCurvesAndSplinesTest.hpp"
#include "Game/Game.hpp"

EasingCurvesAndSplinesTest::EasingCurvesAndSplinesTest(VisualTestType myTestType, Game* game)
	: VisualTest(myTestType, game)
{
	Vec2 viewportMins = Vec2(GetScreenWidth() * .01f, GetScreenHeight() * .02f);
	Vec2 viewportMaxs = Vec2(GetScreenWidth() * .99f, GetScreenHeight() * .95f);
	m_totalViewport = AABB2(viewportMins, viewportMaxs);
	m_splinePane = m_totalViewport.GetFractionOfBox(Vec2(0.f, 0.f), Vec2(1.f, .48f));
	m_easingFunctionPane = m_totalViewport.GetFractionOfBox(Vec2(0.f, .52f), Vec2(.48f, 1.f));
	m_bezierCurvePane = m_totalViewport.GetFractionOfBox(Vec2(.5f, .52f), Vec2(1.f, 1.f));
	m_easingFunctionBox = m_easingFunctionPane.GetFractionOfBox(Vec2(.275f, .1f), Vec2(.725f, 1.f));
	AddVertsForAABB2D(paneVerts, m_splinePane, Rgba8(255, 0, 0, 125));
	AddVertsForAABB2D(paneVerts, m_easingFunctionPane, Rgba8(255, 0, 0, 125));
	AddVertsForAABB2D(paneVerts, m_bezierCurvePane, Rgba8(255, 0, 0, 125));
	AddVertsForAABB2D(m_easingFunctionBoxVerts, m_easingFunctionBox, Rgba8(30, 30, 60));
	m_easingFunctionNameBox = m_easingFunctionPane.GetFractionOfBox(Vec2(0.f, 0.f), Vec2(1.f, .1f));

	m_easingFuncitons.push_back(EasingFunctionData("Identity", &Identity));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStart2", &SmoothStart2));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStart3", &SmoothStart3));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStart4", &SmoothStart4));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStart5", &SmoothStart5));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStart6", &SmoothStart6));

	m_easingFuncitons.push_back(EasingFunctionData("SmoothStop2", &SmoothStop2));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStop3", &SmoothStop3));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStop4", &SmoothStop4));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStop5", &SmoothStop5));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStop6", &SmoothStop6));

	m_easingFuncitons.push_back(EasingFunctionData("SmoothStep3", &SmoothStep3));
	m_easingFuncitons.push_back(EasingFunctionData("SmoothStep5", &SmoothStep5));

	m_easingFuncitons.push_back(EasingFunctionData("Hesitate3", &Hesitate3));
	m_easingFuncitons.push_back(EasingFunctionData("Hesitate5", &Hesitate5));

	m_easingFuncitons.push_back(EasingFunctionData("CustomFunkyEasingFunction", &CustomFunkyEasingFunction));
	Vec2 startPos = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	Vec2 controlPoint1 = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	Vec2 controlPoint2 = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	Vec2 endPos = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	m_cubicBezier = CubicBezierCurve2D(startPos, controlPoint1, controlPoint2, endPos);

	float xVariation = 20.f;
	int numPositions = g_randGen->RollRandomIntInRange(4, 8);
	float splinePaneStartX = m_splinePane.m_mins.x + xVariation;
	float splinePaneEndX = m_splinePane.m_maxs.x - xVariation;
	float splinePaneWidth = splinePaneEndX - splinePaneStartX;

	std::vector<Vec2> positions;
	for (int i = 0; i < numPositions; i++)
	{
		Vec2 currPos;
		currPos.x = splinePaneStartX + splinePaneWidth * ((float)i / (float)(numPositions - 1));
		currPos.x += g_randGen->RollRandomFloatInRange(-xVariation, xVariation);
		currPos.y = g_randGen->RollRandomFloatInRange(m_splinePane.m_mins.y, m_splinePane.m_maxs.y);
		positions.push_back(currPos);
	}
	m_catmullRomSpline = CatmullRomSpline(positions);
}

void EasingCurvesAndSplinesTest::Update(float deltaSeconds)
{
	CheckForInputs();
	m_currT += deltaSeconds;
	m_currCatmullT += deltaSeconds;
	if (m_currT > m_maxT)
	{
		m_currT = 0.f;
	}
	if (m_currCatmullT > m_maxT * (float)(m_catmullRomSpline.GetPositions().size() - 1))
	{
		m_currCatmullT = 0.f;
	}
	UpdateEasingFunction();
	UpdateBezierCurve();
	UpdateCatmullRomSpline();
}

void EasingCurvesAndSplinesTest::Render()
{
	g_theRenderer->BeginCamera(m_game->m_screenCamera);
	g_theRenderer->BindTexture(g_bitmapFont->GetTexture());

	std::vector<Vertex_PCU> textVerts;
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .95f), 1.75f,
		Stringf("F8 to randomize; W/E = prev/next Easing function; N/M = curve subdivisions (%d), hold T = slow", m_curveSubdivisoins), Rgba8(100, 255, 200));
	g_bitmapFont->AddVertsForText2D(textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .975f), 1.75f, "(Mode F6/F7 for prev/next): Easing, Curves, Splines (2D)", Rgba8(255, 250, 100));

	g_theRenderer->DrawVertexArray(textVerts.size(), textVerts.data());
	
	//debug pane
	if (g_theApp->m_debugMode)
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(paneVerts.size(), paneVerts.data());
	}

	//easing function
	g_theRenderer->BindTexture(g_bitmapFont->GetTexture());
	std::vector<Vertex_PCU> nameVerts;
	g_bitmapFont->AddVertsForTextInBox2D(nameVerts, m_easingFunctionNameBox, 3.f, m_easingFuncitons[m_currEasingFunctionIdx].m_name, Rgba8::WHITE, 1.f, Vec2(.5f, 0.f));
	g_theRenderer->DrawVertexArray(nameVerts.size(), nameVerts.data());

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_easingFunctionBoxVerts.size(), m_easingFunctionBoxVerts.data());

	std::vector<Vertex_PCU> pointVerts;
	std::vector<Vertex_PCU> easingFunctionVerts;

	AddVertsForDisc2D(pointVerts, m_easingFunctionCurrPoint, .5f, Rgba8::WHITE);
	AddVertsForEasingFunction(easingFunctionVerts, m_easingFuncitons[m_currEasingFunctionIdx].m_easingFunctionPtr,
		m_easingFunctionBox.m_mins, m_easingFunctionBox.m_maxs, .1f, Rgba8::GREEN, m_curveSubdivisoins);

	//y line
	AddVertsForLine2D(easingFunctionVerts, Vec2(m_easingFunctionBox.m_mins.x, m_easingFunctionCurrPoint.y), Vec2(m_easingFunctionCurrPoint.x, m_easingFunctionCurrPoint.y),
		.1f, Rgba8(125,125,125));
	//x line
	AddVertsForLine2D(easingFunctionVerts, Vec2(m_easingFunctionCurrPoint.x, m_easingFunctionBox.m_mins.y), Vec2(m_easingFunctionCurrPoint.x, m_easingFunctionCurrPoint.y),
		.1f, Rgba8(125, 125, 125));
	g_theRenderer->DrawVertexArray(easingFunctionVerts.size(),  easingFunctionVerts.data());
	g_theRenderer->DrawVertexArray(pointVerts.size(), pointVerts.data());

	//bezier curve
	DrawCubicBezier();

	//catmull rom
	DrawCatmullRomSpline();

	g_theRenderer->EndCamera(m_game->m_screenCamera);
}

void EasingCurvesAndSplinesTest::InitializeTest()
{
	g_theInput->SetCursorMode(false, false);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	RandomizeTest();
}

void EasingCurvesAndSplinesTest::RandomizeTest()
{
	m_currEasingFunctionIdx = g_randGen->RollRandomIntInRange(0, (int)m_easingFuncitons.size() - 1);
	float xVariation = 20.f;
	int numPositions = g_randGen->RollRandomIntInRange(4, 8);
	float splinePaneStartX = m_splinePane.m_mins.x + xVariation;
	float splinePaneEndX = m_splinePane.m_maxs.x - xVariation;
	float splinePaneWidth = splinePaneEndX - splinePaneStartX;

	Vec2 startPos = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	Vec2 controlPoint1 = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	Vec2 controlPoint2 = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	Vec2 endPos = g_randGen->RollRandomVec2InRange(m_bezierCurvePane.m_mins, m_bezierCurvePane.m_maxs);
	m_cubicBezier = CubicBezierCurve2D(startPos, controlPoint1, controlPoint2, endPos);

	std::vector<Vec2> positions;
	for (int i = 0; i < numPositions; i++)
	{
		Vec2 currPos;
		currPos.x = splinePaneStartX + splinePaneWidth * ((float)i / (float)(numPositions - 1));
		currPos.x += g_randGen->RollRandomFloatInRange(-xVariation, xVariation);
		currPos.y = g_randGen->RollRandomFloatInRange(m_splinePane.m_mins.y, m_splinePane.m_maxs.y);
		positions.push_back(currPos);
	}
	m_catmullRomSpline = CatmullRomSpline(positions);
}

void EasingCurvesAndSplinesTest::UpdateEasingFunction()
{
	float currTNormalized = m_currT / m_maxT;
	m_currEasingFuncitonT = m_easingFuncitons[m_currEasingFunctionIdx].m_easingFunctionPtr(currTNormalized);
	m_easingFunctionCurrPoint.x = Lerp(m_easingFunctionBox.m_mins.x, m_easingFunctionBox.m_maxs.x, currTNormalized);
	m_easingFunctionCurrPoint.y = Lerp(m_easingFunctionBox.m_mins.y, m_easingFunctionBox.m_maxs.y, m_currEasingFuncitonT);
}

void EasingCurvesAndSplinesTest::UpdateCatmullRomSpline()
{
	float currTAtCorrectSpeed = m_currCatmullT / m_maxT;
	m_catmullPos = m_catmullRomSpline.EvaluateAtParametric(currTAtCorrectSpeed);

	float currTNormalized = currTAtCorrectSpeed /  (float)(m_catmullRomSpline.GetPositions().size() - 1);
	float approxDistance = m_catmullRomSpline.GetApproximateLength(m_curveSubdivisoins) * currTNormalized;
	m_catmullGreenPos = m_catmullRomSpline.EvaluateAtApproximateDistance(approxDistance, m_curveSubdivisoins);
}

void EasingCurvesAndSplinesTest::DrawCatmullRomSpline()
{

	float lineThickness = .15f;
	std::vector<Vertex_PCU> catmullRomSplineVerts;

	std::vector<Vec2> positions = m_catmullRomSpline.GetPositions();
	std::vector<Vec2> velocities = m_catmullRomSpline.GetVelocities();

	for (size_t i = 0; i < positions.size() - 1; i++)
	{
		AddVertsForLine2D(catmullRomSplineVerts, positions[i], positions[i + 1], lineThickness, DARK_BLUE);
	}

	AddVertsForCatmullRomSpline(catmullRomSplineVerts, m_catmullRomSpline, .2f, DARK_GREY, 64);
	AddVertsForCatmullRomSpline(catmullRomSplineVerts, m_catmullRomSpline, .2f, Rgba8::GREEN, m_curveSubdivisoins);

	for (int i = 1; i < (int)positions.size() - 1; i++)
	{
		AddVertsForArrow2D(catmullRomSplineVerts, positions[i], positions[i] + velocities[i], .5f, .15f, Rgba8::RED);
	}

	for (size_t i = 0; i < positions.size(); i++)
	{
		AddVertsForDisc2D(catmullRomSplineVerts, positions[i], .75f, LIGHT_BLUE);
	}

	AddVertsForDisc2D(catmullRomSplineVerts, m_catmullPos, .75f, Rgba8::WHITE);
	AddVertsForDisc2D(catmullRomSplineVerts, m_catmullGreenPos, .75f, Rgba8::GREEN);

	g_theRenderer->DrawVertexArray(catmullRomSplineVerts.size(), catmullRomSplineVerts.data());
}

void EasingCurvesAndSplinesTest::UpdateBezierCurve()
{
	float currTNormalized = m_currT / m_maxT;
	m_bezierCurveCurrPoint = m_cubicBezier.EvaluateAtParametric(currTNormalized);

	float approxDistance = m_cubicBezier.GetApproximateLength(m_curveSubdivisoins) * currTNormalized;
	m_bezierCurveGreenPoint = m_cubicBezier.EvaluateAtApproximateDistance(approxDistance, m_curveSubdivisoins);
}


void EasingCurvesAndSplinesTest::CheckForInputs()
{
	if (g_theInput->WasKeyJustPressed('W'))
	{
		m_currEasingFunctionIdx--;
		if (m_currEasingFunctionIdx < 0)
		{
			m_currEasingFunctionIdx = (int)m_easingFuncitons.size() - 1;
		}
	}

	if (g_theInput->WasKeyJustPressed('E'))
	{
		m_currEasingFunctionIdx++;
		if (m_currEasingFunctionIdx >= (int)m_easingFuncitons.size())
		{
			m_currEasingFunctionIdx = 0;
		}
	}

	if (g_theInput->WasKeyJustPressed('N'))
	{
		m_curveSubdivisoins = m_curveSubdivisoins / 2;
		if (m_curveSubdivisoins < 1)
		{
			m_curveSubdivisoins = 1;
		}
	}

	if (g_theInput->WasKeyJustPressed('M'))
	{
		m_curveSubdivisoins *= 2;
	}
}

void EasingCurvesAndSplinesTest::DrawCubicBezier()
{
	std::vector<Vertex_PCU> cubicBezierVerts;
	g_theRenderer->BindTexture(nullptr);
	float controlPointSize = .75f;
	float lineThickness = .15f;


	//connecting lines
	AddVertsForLine2D(cubicBezierVerts, m_cubicBezier.m_startPos, m_cubicBezier.m_guidePos1, lineThickness, DARK_BLUE);
	AddVertsForLine2D(cubicBezierVerts, m_cubicBezier.m_guidePos1, m_cubicBezier.m_guidePos2, lineThickness, DARK_BLUE);
	AddVertsForLine2D(cubicBezierVerts, m_cubicBezier.m_guidePos2, m_cubicBezier.m_endPos, lineThickness, DARK_BLUE);


	AddVertsForCubicBezier(cubicBezierVerts, m_cubicBezier, .2f, DARK_GREY, 64);
	AddVertsForCubicBezier(cubicBezierVerts, m_cubicBezier, .2f, Rgba8::GREEN, m_curveSubdivisoins);

	//control points
	AddVertsForDisc2D(cubicBezierVerts, m_cubicBezier.m_startPos, controlPointSize, LIGHT_BLUE);
	AddVertsForDisc2D(cubicBezierVerts, m_cubicBezier.m_guidePos1, controlPointSize, LIGHT_BLUE);
	AddVertsForDisc2D(cubicBezierVerts, m_cubicBezier.m_guidePos2, controlPointSize, LIGHT_BLUE);
	AddVertsForDisc2D(cubicBezierVerts, m_cubicBezier.m_endPos, controlPointSize, LIGHT_BLUE);

	AddVertsForDisc2D(cubicBezierVerts, m_bezierCurveCurrPoint, controlPointSize, Rgba8::WHITE);
	AddVertsForDisc2D(cubicBezierVerts, m_bezierCurveGreenPoint, controlPointSize, Rgba8::GREEN);

	g_theRenderer->DrawVertexArray(cubicBezierVerts.size(), cubicBezierVerts.data());
}


EasingFunctionData::EasingFunctionData(std::string name, EasingFunctionPtr easingFunctionPtr)
	: m_name(name)
	, m_easingFunctionPtr(easingFunctionPtr)
{
}

float CustomFunkyEasingFunction(float t)
{
	return Lerp(SmoothStep5(t), Hesitate5(t), t);
}

float Identity(float t)
{
	return t;
}
