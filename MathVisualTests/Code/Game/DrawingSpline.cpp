#include "DrawingSpline.hpp"
#include "Game/Game.hpp"

FloatCurve floatCurve;

DrawingSpline::DrawingSpline(VisualTestType myTestType, Game* game)
	: VisualTest(myTestType, game)
{
	Vec2 viewportMins = Vec2(GetScreenWidth() * .35f, GetScreenHeight() * .2f);
	Vec2 viewportMaxs = Vec2(GetScreenWidth() * .65f, GetScreenHeight() * .8f);


	m_demoStartPos = Vec2(GetScreenWidth() * .1f, GetScreenHeight() * .5f);
	m_demoEndPos = Vec2(GetScreenWidth() * .25f, GetScreenHeight() * .5f);
	m_demoCurrPos = m_demoStartPos;

	m_demoStartScale = Vec2::ZERO;
	m_demoEndScale = Vec2::ONE;
	m_demoCurrScale = m_demoStartScale;

	m_demoStartColor = Rgba8::WHITE;
	m_demoEndColor = Rgba8::RED;
	m_demoCurrColor = m_demoStartColor;

	m_demoStartRotation = 0.f;
	m_demoEndRotation = 360.f;
	m_demoCurrRotation = m_demoStartRotation;

	m_totalViewport = AABB2(viewportMins, viewportMaxs);
	g_theEventSystem->SubscribeEventCallbackFunction("SaveCurve", Event_SaveCurveToFile);
	g_theEventSystem->SubscribeEventCallbackFunction("LoadCurve", Event_LoadCurveFromFile);
	g_theEventSystem->SubscribeEventCallbackFunction("Start0End1", Event_Start0End1);
	g_theEventSystem->SubscribeEventCallbackFunction("SetPointPos", Event_SetPointPos);
	RandomizeTest();
}

void DrawingSpline::Update(float deltaSeconds)
{
	HandleInput();
	m_currFloatCurveT += deltaSeconds;
	if (m_currFloatCurveT > m_maxT)
	{
		m_currFloatCurveT = 0.f;
	}
	UpdateFloatCurve();
}

void DrawingSpline::Render()
{
	g_theRenderer->BeginCamera(m_game->m_screenCamera);

	//catmull rom
	DrawFloatCurve();

	g_theRenderer->EndCamera(m_game->m_screenCamera);
}

void DrawingSpline::InitializeTest()
{
	g_theInput->SetCursorMode(false, false);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	RandomizeTest();
}

void DrawingSpline::RandomizeTest()
{
	std::vector<FloatCurvePoint> curvePoints;
	FloatCurvePoint startPoint;
	startPoint.m_position = Vec2::ZERO;
	startPoint.m_incomingMode = InterpolationMode::LINEAR;
	startPoint.m_outgoingMode = InterpolationMode::LINEAR;
	curvePoints.push_back(startPoint);

	FloatCurvePoint endPoint;
	endPoint.m_position = Vec2::ONE;
	endPoint.m_incomingMode = InterpolationMode::LINEAR;
	endPoint.m_outgoingMode = InterpolationMode::LINEAR;
	curvePoints.push_back(endPoint);

	floatCurve = FloatCurve(curvePoints);
}

void DrawingSpline::HandleInput()
{
	m_showIncomingVelocities = g_theInput->IsKeyDown('I');
	m_showOutgoingvelocities = g_theInput->IsKeyDown('U');
	Vec2 mousePos = g_theInput->GetCursorNormalizedPosition() * Vec2(GetScreenWidth(), GetScreenHeight());
	if (g_theInput->WasKeyJustPressed('W'))
	{
		floatCurve.WriteCurveToXML("Test", "Data/Curves/TestCurve.xml");
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		std::vector<Vec2> positions = floatCurve.GetPositions();
		for (int i = 0; i < (int)positions.size(); i++)
		{
			if (GetDistance2D(mousePos,m_totalViewport.GetPointAtUV(positions[i])) < m_pointRadius)
			{
				m_selectedPosIdx = i;
			}
		}
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE))
	{
		m_selectedPosIdx = -1;
	}
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		if (m_selectedPosIdx != -1)
		{
			floatCurve.SetPosition(m_selectedPosIdx, m_totalViewport.GetUVForPoint(mousePos));
			if (g_theInput->WasKeyJustPressed('D'))
			{
				floatCurve.RemovePoint(m_totalViewport.GetUVForPoint(mousePos));
				m_selectedPosIdx = -1;
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		FloatCurvePoint pointToAdd;
		pointToAdd.m_position = m_totalViewport.GetUVForPoint(mousePos);
		pointToAdd.m_incomingMode = InterpolationMode::LINEAR;
		pointToAdd.m_outgoingMode = InterpolationMode::LINEAR;
		floatCurve.AddPoint(pointToAdd);
	}
}

void DrawingSpline::UpdateFloatCurve()
{
	float currTAtCorrectSpeed = m_currFloatCurveT / m_maxT;
	m_floatCurvePos.x = Lerp(m_totalViewport.m_mins.x, m_totalViewport.m_maxs.x, currTAtCorrectSpeed);
	m_floatCurvePos.y = Lerp(m_totalViewport.m_mins.y, m_totalViewport.m_maxs.y, floatCurve.EvaluateAt(currTAtCorrectSpeed));

	m_demoCurrPos = Vec2::Lerp(m_demoStartPos, m_demoEndPos, floatCurve.EvaluateAt(currTAtCorrectSpeed));
	m_demoCurrScale = Vec2::Lerp(m_demoStartScale, m_demoEndScale, floatCurve.EvaluateAt(currTAtCorrectSpeed));
	m_demoCurrColor = LerpColor(m_demoStartColor, m_demoEndColor, floatCurve.EvaluateAt(currTAtCorrectSpeed));
	m_demoCurrRotation = Lerp(m_demoStartRotation, m_demoEndRotation, floatCurve.EvaluateAt(currTAtCorrectSpeed));
}

void DrawingSpline::DrawFloatCurve()
{
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	float lineThickness = .15f;
	std::vector<Vertex_PCU> curveVerts;
	AddVertsForAABB2D(curveVerts, m_totalViewport, DARK_GREY);

	std::vector<Vec2> positions = floatCurve.GetPositions();
	for (size_t i = 0; i < positions.size() - 1; i++)
	{
		AddVertsForLine2D(curveVerts, m_totalViewport.GetPointAtUV(positions[i]), m_totalViewport.GetPointAtUV(positions[i + 1]), lineThickness, DARK_BLUE);
	}

	floatCurve.AddVertsForCurve(curveVerts, .2f, m_totalViewport, Rgba8::GREEN, 64);

	for (size_t i = 0; i < positions.size(); i++)
	{
		AddVertsForDisc2D(curveVerts, m_totalViewport.GetPointAtUV(positions[i]), m_pointRadius, LIGHT_BLUE);
	}

	AddVertsForDisc2D(curveVerts, m_floatCurvePos, .75f, Rgba8::WHITE);
	g_theRenderer->DrawVertexArray(curveVerts.size(), curveVerts.data());

	float demoDiscRadius = 5.f;
	std::vector<Vertex_PCU> translationVerts;
	//draw demos
	AddVertsForDisc2D(translationVerts, m_demoCurrPos, demoDiscRadius, Rgba8::WHITE);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(translationVerts.size(), translationVerts.data());

	
	std::vector<Vertex_PCU> rotationVerts;
	Vec2 rotationPosCenter(GetScreenWidth() * .125f, GetScreenHeight() * .3f);
	AddVertsForAABB2D(rotationVerts, AABB2(-Vec2::ONE * demoDiscRadius, Vec2::ONE * demoDiscRadius), Rgba8::WHITE);
	Mat44 rotationMatrix;
	rotationMatrix.AppendTranslation2D(rotationPosCenter);
	rotationMatrix.AppendZRotation(m_demoCurrRotation);
	g_theRenderer->SetModelConstants(rotationMatrix);
	g_theRenderer->DrawVertexArray(rotationVerts.size(), rotationVerts.data());
	

	std::vector<Vertex_PCU> scaleVerts;
	Vec2 scalePosCenter(GetScreenWidth() * .125f, GetScreenHeight() * .7f);
	AddVertsForDisc2D(scaleVerts, Vec2::ZERO, demoDiscRadius, Rgba8::WHITE);
	Mat44 scaleMatrix;
	scaleMatrix.AppendTranslation2D(scalePosCenter);
	scaleMatrix.AppendScaleNonUniform2D(m_demoCurrScale);
	g_theRenderer->SetModelConstants(scaleMatrix);
	g_theRenderer->DrawVertexArray(scaleVerts.size(), scaleVerts.data());

	std::vector<Vertex_PCU> colorVerts;
	Vec2 colorPosCenter(GetScreenWidth() * .125f, GetScreenHeight() * .9f);
	AddVertsForDisc2D(colorVerts, colorPosCenter, demoDiscRadius, m_demoCurrColor);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray(colorVerts.size(), colorVerts.data());
}

bool Event_SaveCurveToFile(EventArgs& args)
{
	std::string curveName = args.GetValue("name", "");
	if (curveName == "")
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Invocation: SaveCurve name=CurveName");
	}
	std::string filePath = "Data/Curves/" + curveName + ".xml";
	floatCurve.WriteCurveToXML(curveName, filePath);
	return false;
}

bool Event_LoadCurveFromFile(EventArgs& args)
{
	std::string path = args.GetValue("filePath", "");
	if (path == "")
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Invocation: LoadFile filePath=CurveFilePath");
	}
	XmlDocument gameConfigDocument;
	GUARANTEE_OR_DIE(gameConfigDocument.LoadFile(path.c_str()) == 0, Stringf("Failed to load curves file %s", path.c_str()));
	XmlElement* rootElement = gameConfigDocument.FirstChildElement("FloatCurve");
	GUARANTEE_OR_DIE(rootElement != nullptr, Stringf("Could not get the root GameConfig element for curves from %s", path.c_str()));
	floatCurve.ClearPoints();
	floatCurve.LoadFromXmlElement(*rootElement);
	return true;
}

bool Event_Start0End1(EventArgs& args)
{
	UNUSED(args);
	floatCurve.Start0End1();
	return false;
}

bool Event_SetPointPos(EventArgs& args)
{
	int index = args.GetValue("index", -1);
	if (index == -1)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Invocation: SetPointPos index=idx pos=position");
		return false;
	}
	Vec2 notFoundVector(-99.f, -99.f);
	Vec2 pos = args.GetValue("pos", notFoundVector);
	if (pos == notFoundVector)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "Invocation: SetPointPos index=idx pos=position");
		return false;
	}
	floatCurve.SetPosition(index, pos);
	return false;
}

