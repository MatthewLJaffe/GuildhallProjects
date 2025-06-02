#include "ConvexSceneEditorTest.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"

ConvexSceneEditorTest::ConvexSceneEditorTest(VisualTestType myTestType, Game* game)
	: VisualTest(myTestType, game)
{
	m_prevMousePos = m_mousePos;
	g_theEventSystem->SubscribeEventCallbackFunction("SaveConvexScene", ConvexSceneEditorTest::Event_SaveConvexScene);
	g_theEventSystem->SubscribeEventCallbackFunction("LoadConvexScene", ConvexSceneEditorTest::Event_LoadConvexScene);
}

void ConvexSceneEditorTest::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	DebugAddMessage("F1=DebugDrawDiscs F2=DebugDrawPartioning F3=DrawMode F4=ToggleBoundingDiscs F5=ToggleBitBuckets F8=Randomize", 1.5f, 0.f);
	DebugAddMessage(Stringf("RMB/LMB=RayStart/End Space=RayMove Q/E=RotateObject L/K=ScaleObject LMB=DragObject"), 1.5f, 0.f);
	DebugAddMessage(Stringf("%d convexShapes (N/M To halve/double); T=Test with %d random rays (Y/U to halve/double)", m_sceneObjects.size(), m_numInvisibleRaycasts), 1.5f, 0.f);
	Vec2 normalizedMousePos = g_theWindow->GetNormalizedCursorPos();
	Vec2 screenBounds = m_game->m_screenCamera.GetOrthoDimensions();
	m_gridCellWidth = screenBounds.x / (float)m_gridWidth;

	m_mousePos = Vec2(normalizedMousePos.x * screenBounds.x, normalizedMousePos.y * screenBounds.y);

	//move arrow
	Vec2 moveDir(0, 0);
	bool rayChaned = false;
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
		m_controlledRay.m_start += moveDir * m_startMoveSpeed * deltaSeconds;
		rayChaned = true;
	}
	
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 rayEnd = m_controlledRay.m_start + m_controlledRay.m_normal * m_controlledRay.m_distance;
		m_controlledRay.m_start = m_mousePos;
		m_controlledRay.m_normal = rayEnd - m_controlledRay.m_start;
		m_controlledRay.m_distance = m_controlledRay.m_normal.GetLength();
		m_controlledRay.m_normal = m_controlledRay.m_normal.GetNormalized();
		rayChaned = true;
	}
	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 rayEndPos = m_mousePos;
		m_controlledRay.m_normal = rayEndPos - m_controlledRay.m_start;
		m_controlledRay.m_distance = m_controlledRay.m_normal.GetLength();
		m_controlledRay.m_normal = m_controlledRay.m_normal.GetNormalized();
		rayChaned = true;
	}
	if (rayChaned)
	{
		CreateRayMask(m_controlledRay);
	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		HalveScene();
	}
	else if (g_theInput->WasKeyJustPressed('M'))
	{
		DoubleScene();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_debugDrawDiscs = !m_debugDrawDiscs;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_debugDrawPartitioning = !m_debugDrawPartitioning;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_translucentDraw = !m_translucentDraw;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_useBoundingDisc = !m_useBoundingDisc;
		if (m_useBoundingDisc)
		{
			DebugAddMessage("Bounding Disc: Enabled", 1.5f, 5.f);
		}
		else
		{
			DebugAddMessage("Bounding Disc: Disabled", 1.5f, 5.f);
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_useBitBuckes = !m_useBitBuckes;
		if (m_useBitBuckes)
		{
			DebugAddMessage("BitBuckets: Enabled", 1.5f, 5.f);
		}
		else
		{
			DebugAddMessage("BitBuckets: Disabled", 1.5f, 5.f);
		}
	}
	if (g_theInput->WasKeyJustPressed('T'))
	{
		TestInvisibleRays();
	}
	if (g_theInput->WasKeyJustPressed('U'))
	{
		DoubleRays();
	}
	if (g_theInput->WasKeyJustPressed('Y'))
	{
		HalfRays();
	}
	if (m_hilightedObjectIndex == -1 || m_hilightedObjectIndex < m_sceneObjects.size() && !IsPointInsideConvexHull2D(m_mousePos, m_sceneObjects[m_hilightedObjectIndex].m_convexHull) 
		&& !g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		bool objectHilighted = false;
		for (int i = 0; i < m_sceneObjects.size(); i++)
		{
			if (IsPointInsideConvexHull2D(m_mousePos, m_sceneObjects[i].m_convexHull))
			{
				m_hilightedObjectIndex = i;
				objectHilighted = true;
				break;
			}
		}
		if (!objectHilighted)
		{
			m_hilightedObjectIndex = -1;
		}
	}
	if (m_hilightedObjectIndex != -1 && m_hilightedObjectIndex < (int)m_sceneObjects.size())
	{
		if (g_theInput->IsKeyDown('Q'))
		{
			RotateObject(90.f * deltaSeconds);
		}
		else if (g_theInput->IsKeyDown('E'))
		{
			RotateObject(-90.f * deltaSeconds);
		}
		if (g_theInput->IsKeyDown('K'))
		{
			ScaleObject(1.f - (deltaSeconds));
		}
		if (g_theInput->IsKeyDown('L'))
		{
			ScaleObject(1.f + (deltaSeconds));
		}
		if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
		{
			MoveObjectWithMouse();
		}
	}
	m_closestRaycast = RaycastResult2D();

	m_objectHitIndex = -1;
	for (int i = 0; i < m_sceneObjects.size(); i++)
	{
		if (m_useBitBuckes)
		{
			if ((m_controlledRay.m_arrowMask & m_sceneObjects[i].m_sceneMask) == 0)
			{
				continue;
			}
		}
		if (m_useBoundingDisc)
		{
			RaycastResult2D result = RaycastVsDisc2D(m_controlledRay.m_start, m_controlledRay.m_normal, m_controlledRay.m_distance,
				m_sceneObjects[i].m_boundingDisc.m_centerPosition, m_sceneObjects[i].m_boundingDisc.m_radius);
			if (!result.m_didImpact)
			{
				continue;
			}
		}
		RaycastResult2D currResult = RaycastVSConvexHull2D(m_controlledRay.m_start, m_controlledRay.m_normal, m_controlledRay.m_distance, m_sceneObjects[i].m_convexHull);
		if (currResult.m_didImpact)
		{
			if (!m_closestRaycast.m_didImpact || currResult.m_impactDist < m_closestRaycast.m_impactDist)
			{
				m_closestRaycast = currResult;
				m_objectHitIndex = i;
			}
		}
	}
	m_prevMousePos = m_mousePos;
}

void ConvexSceneEditorTest::Render()
{
	Rgba8 missFillColor = Rgba8(0, 0, 100);
	Rgba8 hitFillColor = Rgba8(50, 50, 200);

	Rgba8 missEdgeColor = Rgba8(30, 50, 150);
	Rgba8 hitEdgeColor = Rgba8(80, 110, 255);

	std::vector<Vertex_PCU> sceneObjectVerts;
	g_theRenderer->BeginCamera(m_game->m_screenCamera);

	if (m_translucentDraw)
	{
		missFillColor = Rgba8(0, 0, 100, 100);
		hitFillColor = Rgba8(0, 0, 150, 100);

		for (int i = 0; i < m_sceneObjects.size(); i++)
		{
			m_sceneObjects[i].AddVertsForConvexPoly(sceneObjectVerts, missFillColor);
			m_sceneObjects[i].AddVertsForConvexPolyEdges(sceneObjectVerts, .2f, missEdgeColor);
		}
	}
	else
	{
		for (int i = 0; i < m_sceneObjects.size(); i++)
		{
			m_sceneObjects[i].AddVertsForConvexPolyEdges(sceneObjectVerts, .2f, missEdgeColor);
		}
		for (int i = 0; i < m_sceneObjects.size(); i++)
		{
			m_sceneObjects[i].AddVertsForConvexPoly(sceneObjectVerts, missFillColor);
		}
	}

	Vec2 arrowEndPos = m_controlledRay.m_start + m_controlledRay.m_normal * m_controlledRay.m_distance;
	if (m_closestRaycast.m_didImpact)
	{
		m_sceneObjects[m_objectHitIndex].AddVertsForConvexPoly(sceneObjectVerts, hitFillColor);
		m_sceneObjects[m_objectHitIndex].AddVertsForConvexPolyEdges(sceneObjectVerts,.2f, hitEdgeColor);
		Vec2 normalStart = m_closestRaycast.m_impactPos;
		Vec2 normalEnd = m_closestRaycast.m_impactPos + m_closestRaycast.m_impactNormal * 4.f;
		AddVertsForArrow2D(sceneObjectVerts, m_controlledRay.m_start, arrowEndPos, 1.f, .1f, Rgba8(127, 127, 127));
		AddVertsForArrow2D(sceneObjectVerts, m_controlledRay.m_start, m_closestRaycast.m_impactPos, 1.f, .1f, Rgba8::RED);
		AddVertsForArrow2D(sceneObjectVerts, normalStart, normalEnd, 1.f, .1f, Rgba8::YELLOW);
		AddVertsForDisc2D(sceneObjectVerts, m_closestRaycast.m_impactPos, .3f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(sceneObjectVerts, m_controlledRay.m_start, arrowEndPos, 1.f, .1f, Rgba8::GREEN);
	}

	if (m_sceneObjects.size() == 1)
	{
		AddVertsForHullPlanesAndRayInterceptPoints(sceneObjectVerts);
	}

	if (m_hilightedObjectIndex != -1 && m_hilightedObjectIndex < (int)m_sceneObjects.size())
	{
		m_sceneObjects[m_hilightedObjectIndex].AddVertsForConvexPoly(sceneObjectVerts, Rgba8(40, 50, 100));
		m_sceneObjects[m_hilightedObjectIndex].AddVertsForConvexPolyEdges(sceneObjectVerts, .2f, Rgba8::WHITE);
	}
	if (m_debugDrawPartitioning)
	{
		AddVertsForGridLines(sceneObjectVerts);
	}
	if (m_debugDrawDiscs)
	{
		AddVertsForBoundingDiscs(sceneObjectVerts);
	}
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->DrawVertexArray(sceneObjectVerts.size(), sceneObjectVerts.data());
	g_theRenderer->EndCamera(m_game->m_screenCamera);
}

void ConvexSceneEditorTest::InitializeTest()
{
	TestBinaryFileLoad();
	TestBinaryFileSave();
	m_sceneObjects.clear();
	for (int i = 0; i < m_numObjects; i++)
	{
		AddRandomConvexObject();
	}
	m_invisibleRays.clear();
	for (int i = 0; i < m_numInvisibleRaycasts; i++)
	{
		AddRandomInvisibleRay();
	}
}

void ConvexSceneEditorTest::RandomizeTest()
{
	InitializeTest();
}

void ConvexSceneEditorTest::AddVertsForHullPlanesAndRayInterceptPoints(std::vector<Vertex_PCU>& verts)
{
	ConvexHull2D const& convexHull = m_sceneObjects[0].m_convexHull;

	for (int i = 0; i < (int)convexHull.m_boundingPlanes.size(); i++)
	{
		Plane2 const& currentPlane = convexHull.m_boundingPlanes[i];
		Vec2 planeMidPos = currentPlane.m_distFromOriginAlongNormal * currentPlane.m_normal;
		Vec2 planeStartPos = planeMidPos + (currentPlane.m_normal.GetRotated90Degrees() * 500.f);
		Vec2 planeEndPos = planeMidPos + (currentPlane.m_normal.GetRotated90Degrees() * -500.f);

		Rgba8 noIntersectColor = Rgba8(5, 200, 50);
		Rgba8 enterColor = Rgba8::MAGENTA;
		Rgba8 exitColor = Rgba8(255, 127, 0);
		RaycastResult2D currRaycastResult = RaycastVsPlane2D(m_controlledRay.m_start, m_controlledRay.m_normal, m_controlledRay.m_distance, currentPlane);
		if (currRaycastResult.m_didImpact)
		{
			float altitude = currentPlane.GetAltitude(m_controlledRay.m_start);
			if (altitude > 0.f)
			{
				AddVertsForDisc2D(verts, currRaycastResult.m_impactPos, .5f, enterColor);
				AddVertsForLine2D(verts, planeStartPos, planeEndPos, .1f, enterColor);
			}
			else
			{
				AddVertsForLine2D(verts, planeStartPos, planeEndPos, .1f, exitColor);
			}
		}
		else
		{
			AddVertsForLine2D(verts, planeStartPos, planeEndPos, .1f, noIntersectColor);
		}
	}
}

void ConvexSceneEditorTest::HalveScene()
{
	if (m_numObjects <= 1)
	{
		return;
	}
	int objectsToErase = m_numObjects / 2;
	m_numObjects /= 2;
	for (int i = 0; i < objectsToErase; i++)
	{
		m_sceneObjects.erase(m_sceneObjects.end() - 1);
	}
}

void ConvexSceneEditorTest::DoubleScene()
{
	int objectsToAdd = m_numObjects;
	m_numObjects *= 2;
	for (int i = 0; i < objectsToAdd; i++)
	{
		AddRandomConvexObject();
	}
}

void ConvexSceneEditorTest::AddRandomConvexObject()
{
	std::vector<Vec2> convexPolyVertexes;
	Vec2 center = RollRandomPositionOnScreen(10.f);
	float radius = g_randGen->RollRandomFloatInRange(4.f, 12.f);
	float currentTheta = 0.f;

	while (currentTheta < 359.999f)
	{
		float thetaStep = g_randGen->RollRandomFloatInRange(15.f, 120.f);
		if (currentTheta + thetaStep > 360.f)
		{
			thetaStep = 360.f - currentTheta;
		}
		Vec2 vertexPos = Vec2::MakeFromPolarDegrees(currentTheta) * radius + center;
		convexPolyVertexes.push_back(vertexPos);
		currentTheta += thetaStep;
	}
	BoundingDisc2D boundingDisc;
	boundingDisc.m_centerPosition = center;
	boundingDisc.m_radius = radius;
	ConvexPolySceneObject sceneObject = ConvexPolySceneObject(convexPolyVertexes, boundingDisc);
	CreateSceneObjectMask(sceneObject);
	m_sceneObjects.push_back(sceneObject);
}

void ConvexSceneEditorTest::AddRandomInvisibleRay()
{
	float dist = g_randGen->RollRandomFloatInRange(5.f, 15.f);
	Vec2 dir = g_randGen->RollRandomNormalizedVec2();
	Vec2 startPos = RollRandomPositionOnScreen(15.f);
	Ray2D rayToAdd;
	rayToAdd.m_distance = dist;
	rayToAdd.m_normal = dir;
	rayToAdd.m_start = startPos;
	m_invisibleRays.push_back(rayToAdd);
}

void ConvexSceneEditorTest::TestInvisibleRays()
{
	m_invisibleRays.clear();
	m_invisibleRays.reserve(m_numInvisibleRaycasts);
	for (int i = 0; i < m_numInvisibleRaycasts; i++)
	{
		AddRandomInvisibleRay();
	}

	double timeBefore = GetCurrentTimeSeconds();
	float averageDistance = 0.f;
	int numImpacts = 0;
	for (int i = 0; i < m_numInvisibleRaycasts; i++)
	{
		if (m_useBitBuckes)
		{
			CreateRayMask(m_invisibleRays[i]);
		}
		for (int objIdx = 0; objIdx < m_numObjects; objIdx++)
		{
			if (m_useBitBuckes)
			{
				if ((m_invisibleRays[i].m_arrowMask & m_sceneObjects[objIdx].m_sceneMask) == 0)
				{
					continue;
				}
			}
			if (m_useBoundingDisc)
			{
				RaycastResult2D discResult = RaycastVsDisc2D(m_invisibleRays[i].m_start, m_invisibleRays[i].m_normal, m_invisibleRays[i].m_distance,
					m_sceneObjects[objIdx].m_boundingDisc.m_centerPosition, m_sceneObjects[objIdx].m_boundingDisc.m_radius);
				if (!discResult.m_didImpact)
				{
					continue;
				}
			}
			RaycastResult2D result = RaycastVSConvexHull2D(m_invisibleRays[i].m_start, m_invisibleRays[i].m_normal, m_invisibleRays[i].m_distance, m_sceneObjects[objIdx].m_convexHull);
			if (result.m_didImpact)
			{
				numImpacts++;
				averageDistance += result.m_impactDist;
			}
		}
	}
	float totalTimeMS = (float)(GetCurrentTimeSeconds() - timeBefore) * 1000.f;
	averageDistance /= (float)(numImpacts);
	DebugAddMessage(Stringf("Test complete Time: %.2f MS Average Distance = %.2f Num Hits=%d", totalTimeMS, averageDistance, numImpacts), 1.5f, 10.f);
}

void ConvexSceneEditorTest::DoubleRays()
{
	m_numInvisibleRaycasts *= 2;
}

void ConvexSceneEditorTest::HalfRays()
{
	if (m_numInvisibleRaycasts <= 1)
	{
		return;
	}
	m_numInvisibleRaycasts /= 2;
}

void ConvexSceneEditorTest::RotateObject(float theta)
{
	ConvexPoly2D& polyToRotate = m_sceneObjects[m_hilightedObjectIndex].m_convexPoly;
	for (int i = 0; i < (int)polyToRotate.m_vertexPositions.size(); i++)
	{
		Vec2 dispToPoint = polyToRotate.m_vertexPositions[i] - m_mousePos;
		dispToPoint = dispToPoint.GetRotatedDegrees(theta);
		polyToRotate.m_vertexPositions[i] = dispToPoint + m_mousePos;
	}
	m_sceneObjects[m_hilightedObjectIndex].m_convexHull = ConvexHull2D(polyToRotate.m_vertexPositions);
	ComputeBoundingDisc(m_sceneObjects[m_hilightedObjectIndex]);
	CreateSceneObjectMask(m_sceneObjects[m_hilightedObjectIndex]);
}

void ConvexSceneEditorTest::ScaleObject(float scaleFactor)
{
	ConvexPoly2D& polyToScale = m_sceneObjects[m_hilightedObjectIndex].m_convexPoly;
	for (int i = 0; i < (int)polyToScale.m_vertexPositions.size(); i++)
	{
		Vec2 dispToPoint = polyToScale.m_vertexPositions[i] - m_mousePos;
		dispToPoint *= scaleFactor;
		polyToScale.m_vertexPositions[i] = m_mousePos + dispToPoint;
	}
	m_sceneObjects[m_hilightedObjectIndex].m_convexHull = ConvexHull2D(polyToScale.m_vertexPositions);
	ComputeBoundingDisc(m_sceneObjects[m_hilightedObjectIndex]);
	CreateSceneObjectMask(m_sceneObjects[m_hilightedObjectIndex]);
}

void ConvexSceneEditorTest::MoveObjectWithMouse()
{
	Vec2 disp = m_mousePos - m_prevMousePos;
	ConvexPoly2D& polyToTranslate = m_sceneObjects[m_hilightedObjectIndex].m_convexPoly;
	for (int i = 0; i < (int)polyToTranslate.m_vertexPositions.size(); i++)
	{
		polyToTranslate.m_vertexPositions[i] += disp;
	}
	m_sceneObjects[m_hilightedObjectIndex].m_boundingDisc.m_centerPosition += disp;
	m_sceneObjects[m_hilightedObjectIndex].m_convexHull = ConvexHull2D(polyToTranslate.m_vertexPositions);
	ComputeBoundingDisc(m_sceneObjects[m_hilightedObjectIndex]);
	CreateSceneObjectMask(m_sceneObjects[m_hilightedObjectIndex]);
}

void ConvexSceneEditorTest::CreateRayMask(Ray2D& rayToMask)
{
	std::vector<unsigned long long> hitCells;
	ImprovedRaycastVsGrid(hitCells, rayToMask.m_start, rayToMask.m_normal, rayToMask.m_distance);
	for (int i = 0; i < hitCells.size(); i++)
	{
		unsigned long long cellNumber = hitCells[i];
		unsigned long long bitToShift = 1;
		rayToMask.m_arrowMask |= bitToShift << cellNumber;
	}

	unsigned long long cellNumber = GetGridNumber(rayToMask.m_start);
	unsigned long long bitToShift = 1;
	rayToMask.m_arrowMask |= bitToShift << cellNumber;

	cellNumber = GetGridNumber(rayToMask.m_start + rayToMask.m_normal * rayToMask.m_distance);
	rayToMask.m_arrowMask |= bitToShift << cellNumber;

}

void ConvexSceneEditorTest::ShittyRaycastVsGrid(std::vector<unsigned long long>& cellsHit, Vec2 const& rayStartPos, Vec2 const& rayNormal, float rayDistance)
{
	Vec2 screenBounds = m_game->m_screenCamera.GetOrthoDimensions();
	for (int y = 0; y < m_gridHeight; y++)
	{
		RaycastResult2D result = RaycastVsLineSegment2D(rayStartPos, rayNormal, rayDistance,
			Vec2(-100, (float)y * m_gridCellHeight), Vec2(screenBounds.x * 2, (float)y * m_gridCellHeight));
		if (result.m_didImpact)
		{
			//add cells to left / right of impact
			unsigned long long gridNumber = GetGridNumber(result.m_impactPos);
			cellsHit.push_back(gridNumber);
			if (gridNumber - 1 >= 0)
			{
				cellsHit.push_back(gridNumber - 1);
			}
		}
	}
	for (int x = 0; x < m_gridWidth; x++)
	{
		RaycastResult2D result = RaycastVsLineSegment2D(rayStartPos, rayNormal, rayDistance, Vec2((float)x * m_gridCellWidth, -100), Vec2((float)x * m_gridCellWidth, screenBounds.y * 2));
		if (result.m_didImpact)
		{
			//add cells to bottom / top of impact
			unsigned long long gridNumber = GetGridNumber(result.m_impactPos);
			cellsHit.push_back(gridNumber);
			if (gridNumber - m_gridWidth >= 0)
			{
				cellsHit.push_back(gridNumber - m_gridWidth);
			}
		}
	}
}

void ConvexSceneEditorTest::ImprovedRaycastVsGrid(std::vector<unsigned long long>& cellsHit, Vec2 const& rayStartPos, Vec2 const& rayNormal, float rayDistance)
{

	//add start pos
	cellsHit.push_back(GetGridNumber(rayStartPos));

	int xStepDir = rayNormal.x > 0 ? 1 : -1;
	int yStepDir = rayNormal.y > 0 ? 1 : -1;
	float distanceLeft = rayDistance;
	Vec2 currPos = rayStartPos;
	IntVec2 currTileCoords(RoundDownToInt(currPos.x / m_gridCellWidth), RoundDownToInt(currPos.y / m_gridCellHeight));

	while (distanceLeft > 0)
	{
		//figure out how long to travel along forward normal to reach next x and y
		int nextX = currTileCoords.x + (xStepDir + 1) / 2;
		float distToNextX = static_cast<float>(nextX * m_gridCellWidth) - currPos.x;
		float forwardLenToNextX = fabsf(distToNextX / rayNormal.x);

		int nextY = currTileCoords.y + (yStepDir + 1) / 2;
		float distToNextY = static_cast<float>(nextY * m_gridCellHeight) - currPos.y;
		float forwardLenToNextY = fabsf(distToNextY / rayNormal.y);

		Vec2 nextStepDisplacment;
		if (forwardLenToNextX < forwardLenToNextY)
		{
			nextStepDisplacment = forwardLenToNextX * rayNormal;
			currTileCoords.x += xStepDir;
		}
		else
		{
			nextStepDisplacment = forwardLenToNextY * rayNormal;
			currTileCoords.y += yStepDir;
		}
		distanceLeft -= nextStepDisplacment.GetLength();
		if (distanceLeft < 0)
		{
			break;
		}
		currPos += nextStepDisplacment;
		//crossing x
		if (forwardLenToNextX < forwardLenToNextY)
		{
			unsigned long long gridNumber = currTileCoords.x + (currTileCoords.y * m_gridWidth);
			cellsHit.push_back(gridNumber);
			if (gridNumber - 1 >= 0)
			{
				cellsHit.push_back(gridNumber - 1);
			}
		}
		//crossing y
		else
		{
			unsigned long long gridNumber = currTileCoords.x + (currTileCoords.y * m_gridWidth);
			cellsHit.push_back(gridNumber);
			if (gridNumber - m_gridWidth >= 0)
			{
				cellsHit.push_back(gridNumber - m_gridWidth);
			}
		}		
	}

	//add end pos
	cellsHit.push_back(GetGridNumber(rayStartPos + rayNormal * rayDistance));

}

void ConvexSceneEditorTest::ComputeBoundingDisc(ConvexPolySceneObject& sceneObject)
{
	Vec2 centerPos;
	for (int i = 0; i < (int)sceneObject.m_convexPoly.m_vertexPositions.size(); i++)
	{
		Vec2 vertexPos = sceneObject.m_convexPoly.m_vertexPositions[i];
		centerPos += vertexPos;
	}
	centerPos /= (float)sceneObject.m_convexPoly.m_vertexPositions.size();

	float radiusLength = 0.f;
	for (int i = 0; i < (int)sceneObject.m_convexPoly.m_vertexPositions.size(); i++)
	{
		Vec2 currentVertPos = sceneObject.m_convexPoly.m_vertexPositions[i];
		float currDistanceSquaredFromCenter = GetDistanceSquared2D(centerPos, currentVertPos);
		if (currDistanceSquaredFromCenter > radiusLength)
		{
			radiusLength = currDistanceSquaredFromCenter;
		}
	}
	radiusLength = sqrtf(radiusLength);
	sceneObject.m_boundingDisc.m_radius = radiusLength;
	sceneObject.m_boundingDisc.m_centerPosition = centerPos;
}

void ConvexSceneEditorTest::CreateSceneObjectMask(ConvexPolySceneObject& sceneObject)
{
	sceneObject.m_sceneMask = 0;
	std::vector<Vec2> const& positions = sceneObject.m_convexPoly.m_vertexPositions;
	AABB2 boundingAABB2(positions[0], positions[0]);

	for (int i = 1; i < (int)positions.size(); i++)
	{
		boundingAABB2.StretchToIncludePoint(positions[i]);
	}

	for (int y = 0; y < m_gridHeight; y++)
	{
		for (int x = 0; x < m_gridWidth; x++)
		{
			Vec2 cellMins = Vec2((float)x * m_gridCellWidth, (float)y * m_gridCellHeight);
			Vec2 cellMaxs = Vec2((float)(x+1) * m_gridCellWidth, (float)(y+1) * m_gridCellHeight);
			AABB2 cellBounds(cellMins, cellMaxs);

			if (DoAABBsOverlap2D(boundingAABB2, cellBounds))
			{
				unsigned long long cellNumber = (unsigned long long)(x+y*m_gridWidth);
				unsigned long long bitToShift = 1;
				sceneObject.m_sceneMask |= bitToShift << cellNumber;
			}
		}
	}	
}

unsigned long long ConvexSceneEditorTest::GetGridNumber(Vec2 const& pos)
{
	int xPos = (int)(pos.x / m_gridCellWidth);
	int yPos = (int)(pos.y / m_gridCellHeight);
	int cellNumber = xPos + yPos * m_gridWidth;
	return (unsigned long long)cellNumber;
}

void ConvexSceneEditorTest::AppendTestFileBufferData(BufferWriter& bufferWriter, BufferEndianMode endianMode)
{
	bufferWriter.SetEndianMode(endianMode);
	bufferWriter.AppendChar('T');
	bufferWriter.AppendChar('E');
	bufferWriter.AppendChar('S');
	bufferWriter.AppendChar('T');
	bufferWriter.AppendByte(2); // Version 2
	bufferWriter.AppendByte((unsigned char)bufferWriter.GetEndianMode());
	bufferWriter.AppendBool(false);
	bufferWriter.AppendBool(true);
	bufferWriter.AppendUint32(0x12345678);
	bufferWriter.AppendInt32(-7); // signed 32-bit int
	bufferWriter.AppendFloat(1.f); // in memory looks like hex: 00 00 80 3F (or 3F 80 00 00 in big endian)
	bufferWriter.AppendDouble(3.1415926535897932384626433832795); // actually 3.1415926535897931 (best it can do)
	bufferWriter.AppendStringZeroTerminated("Hello"); // written with a trailing 0 ('\0') after (6 bytes total)
	bufferWriter.AppendStringAfter32BitLength("Is this thing on?"); // uint 17, then 17 chars (no zero-terminator after)
	bufferWriter.AppendRgba(Rgba8(200, 100, 50, 255)); // four bytes in RGBA order (endian-independent)
	bufferWriter.AppendByte(8); // 0x08 == 8 (byte)
	bufferWriter.AppendRgb(Rgba8(238, 221, 204, 255)); // written as 3 bytes (RGB) only; ignores Alpha
	bufferWriter.AppendByte(9); // 0x09 == 9 (byte)
	bufferWriter.AppendIntVec2(IntVec2(1920, 1080));
	bufferWriter.AppendVec2(Vec2(-0.6f, 0.8f));
	bufferWriter.AppendVertexPCU(Vertex_PCU(Vec3(3.f, 4.f, 5.f), Rgba8(100, 101, 102, 103), Vec2(0.125f, 0.625f)));
}

void ConvexSceneEditorTest::ParseTestFileBufferData(BufferParser& bufferParser, BufferEndianMode endianMode)
{
	// Parse known test file elements
	bufferParser.SetEndianMode(endianMode);
	char fourCC0_T = bufferParser.ParseChar(); // 'T' == 0x54 hex == 84 decimal
	char fourCC1_E = bufferParser.ParseChar(); // 'E' == 0x45 hex == 84 decimal
	char fourCC2_S = bufferParser.ParseChar(); // 'S' == 0x53 hex == 69 decimal
	char fourCC3_T = bufferParser.ParseChar(); // 'T' == 0x54 hex == 84 decimal
	unsigned char version = bufferParser.ParseByte(); // version 2
	unsigned char readEndianMode = bufferParser.ParseByte(); //endian mode
	UNUSED(readEndianMode);
	bool shouldBeFalse = bufferParser.ParseBool(); // written in buffer as byte 0 or 1
	bool shouldBeTrue = bufferParser.ParseBool(); // written in buffer as byte 0 or 1
	unsigned int largeUint = bufferParser.ParseUint32(); // 0x12345678
	int negativeSeven = bufferParser.ParseInt32(); // -7 (as signed 32-bit int)
	float oneF = bufferParser.ParseFloat(); // 1.0f
	double pi = bufferParser.ParseDouble(); // 3.1415926535897932384626433832795 (or as best it can)

	std::string helloString, isThisThingOnString;
	bufferParser.ParseStringZeroTerminated(helloString); // written with a trailing 0 ('\0') after (6 bytes total)
	bufferParser.ParseStringAfter32BitLength(isThisThingOnString); // written as uint 17, then 17 characters (no zero-terminator after)

	Rgba8 rustColor = bufferParser.ParseRgba8(); // Rgba( 200, 100, 50, 255 )
	unsigned char eight = bufferParser.ParseByte(); // 0x08 == 8 (byte)
	Rgba8 seashellColor = bufferParser.ParseRgb(); // Rgba(238,221,204) written as 3 bytes (RGB) only; ignores Alpha
	unsigned char nine = bufferParser.ParseByte(); // 0x09 == 9 (byte)
	IntVec2 highDefRes = bufferParser.ParseIntVec2(); // IntVector2( 1920, 1080 )
	Vec2 normal2D = bufferParser.ParseVec2(); // Vector2( -0.6f, 0.8f )
	Vertex_PCU vertex = bufferParser.ParseVertex_PCU(); // VertexPCU( 3.f, 4.f, 5.f, Rgba(100,101,102,103), 0.125f, 0.625f ) );

	// Validate actual values parsed
	GUARANTEE_OR_DIE(fourCC0_T == 'T', "Messed up parse");
	GUARANTEE_OR_DIE(fourCC1_E == 'E', "Messed up parse");
	GUARANTEE_OR_DIE(fourCC2_S == 'S', "Messed up parse");
	GUARANTEE_OR_DIE(fourCC3_T == 'T', "Messed up parse");
	GUARANTEE_OR_DIE(version == 2, "Messed up parse");
	GUARANTEE_OR_DIE(shouldBeFalse == false, "Messed up parse");
	GUARANTEE_OR_DIE(shouldBeTrue == true, "Messed up parse");
	GUARANTEE_OR_DIE(largeUint == 0x12345678, "Messed up parse");
	GUARANTEE_OR_DIE(negativeSeven == -7, "Messed up parse");
	GUARANTEE_OR_DIE(oneF == 1.f, "Messed up parse");
	GUARANTEE_OR_DIE(pi == 3.1415926535897932384626433832795, "Messed up parse");
	GUARANTEE_OR_DIE(helloString == "Hello", "Messed up parse");
	GUARANTEE_OR_DIE(isThisThingOnString == "Is this thing on?", "Messed up parse");
	GUARANTEE_OR_DIE(rustColor == Rgba8(200, 100, 50, 255), "Messed up parse");
	GUARANTEE_OR_DIE(eight == 8, "Messed up parse");
	GUARANTEE_OR_DIE(seashellColor == Rgba8(238, 221, 204), "Messed up parse");
	GUARANTEE_OR_DIE(nine == 9, "Messed up parse");
	GUARANTEE_OR_DIE(highDefRes == IntVec2(1920, 1080), "Messed up parse");
	GUARANTEE_OR_DIE(normal2D == Vec2(-0.6f, 0.8f), "Messed up parse");
	GUARANTEE_OR_DIE(vertex.m_position == Vec3(3.f, 4.f, 5.f), "Messed up parse");
	GUARANTEE_OR_DIE(vertex.m_color == Rgba8(100, 101, 102, 103), "Messed up parse");
	GUARANTEE_OR_DIE(vertex.m_uvTexCoords == Vec2(0.125f, 0.625f), "Messed up parse");
}

void ConvexSceneEditorTest::TestBinaryFileLoad()
{
	const char* testFilePath = "Data/Save/Test.binary";
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, Stringf("Loading test binary file '%s'...\n", testFilePath));

	// Load from disk
	std::vector<unsigned char> buffer;
	bool success = FileReadToBuffer(buffer, testFilePath);
	if (!success)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, Stringf("FAILED to load file %s\n", testFilePath));
		return;
	}

	// Parse and verify
	BufferParser bufParser(buffer);
	//ERROR_RECOVERABLE(bufParser.GetRemainingSize() == TEST_BUF_SIZE); // should be 208 Bytes total (two copies of the Test buffer content)
	GUARANTEE_OR_DIE(bufParser.GetRemainingSize() == 208, "size not right"); // should be 208 Bytes total (two copies of the Test buffer content)
	ParseTestFileBufferData(bufParser, BufferEndianMode::LITTLE); // First, the test content in little-endian format
	ParseTestFileBufferData(bufParser, BufferEndianMode::BIG); // Second, the same test content again, but in big-endian format
	GUARANTEE_OR_DIE(bufParser.GetRemainingSize() == 0, "Size not right");

	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, Stringf("...successfully read file %s\n", testFilePath));
}

void ConvexSceneEditorTest::TestBinaryFileSave()
{

	const char* testFilePath = "Data/Save/Matthew.binary";
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, Stringf("Saving test binary file '%s'...", testFilePath));


	// Create the test file buffer
	std::vector<unsigned char> buffer;
	buffer.reserve(1000);
	BufferWriter bufWriter(buffer);
	ASSERT_RECOVERABLE(bufWriter.GetTotalSize() == 0, "Total Size mismatch");
	ASSERT_RECOVERABLE(bufWriter.GetAppendedSize() == 0, "Append size mismatch");

	AppendTestFileBufferData(bufWriter, BufferEndianMode::LITTLE); // First, the test content in little-endian format
	AppendTestFileBufferData(bufWriter, BufferEndianMode::BIG);// Second, the same test content again, but in big-endian format

	ASSERT_RECOVERABLE(bufWriter.GetAppendedSize() == 208, "Appended size mismatch"); // should be 208 Bytes total (two copies of the Test buffer content)
	ASSERT_RECOVERABLE(bufWriter.GetTotalSize() == 208, "Total size mismatch");

	// Write to disk
	bool success = WriteBufferToFile(buffer, testFilePath);
	if (success)
	{
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, Stringf("...successfully wrote file %s", testFilePath));
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, Stringf("FAILED to write file %s", testFilePath));
		g_theDevConsole->AddLine(DevConsole::INFO_WARNING, Stringf("(does the folder exist?)"));
	}
}

void ConvexSceneEditorTest::WriteOutGHCS(BufferWriter& bufferWriter, bool skipDisk, bool skipConvexHull, bool skipBitBucket)
{
	//header
	bufferWriter.AppendChar('G');
	bufferWriter.AppendChar('H');
	bufferWriter.AppendChar('C');
	bufferWriter.AppendChar('S');
	bufferWriter.AppendByte(33);
	bufferWriter.AppendByte(1);
	bufferWriter.AppendByte(1);
	bufferWriter.AppendByte((uint8_t)bufferWriter.GetEndianMode());
	bufferWriter.AppendUint32(5);
	bufferWriter.AppendChar('E');
	bufferWriter.AppendChar('N');
	bufferWriter.AppendChar('D');
	bufferWriter.AppendChar('H');

	WriteOutChunk(bufferWriter, 0x01);
	WriteOutChunk(bufferWriter, 0x02);
	if (!skipConvexHull)
	{
		WriteOutChunk(bufferWriter, 0x80);
	}
	if (!skipDisk)
	{
		WriteOutChunk(bufferWriter, 0x81);
	}
	if (!skipBitBucket)
	{
		WriteOutChunk(bufferWriter, 0x88);
	}


	//table of contents
	//make sure header has correct pointer
	bufferWriter.WriteUint32AtOffset(headerTOCPtrOffset, (unsigned int)bufferWriter.GetAppendedSize());
	bufferWriter.AppendChar('G');
	bufferWriter.AppendChar('H');
	bufferWriter.AppendChar('T');
	bufferWriter.AppendChar('C');
	bufferWriter.AppendByte((uint8_t)m_loadedChunks.size());

	for (int i = 0; i < (int)m_loadedChunks.size(); i++)
	{
		bufferWriter.AppendByte(m_loadedChunks[i].m_chunkType);
		bufferWriter.AppendUint32(m_loadedChunks[i].m_chunkHeaderStart);
		//plus 14 because that is the header+footer size
		bufferWriter.AppendUint32(m_loadedChunks[i].m_chunkTotalSize + 14);
	}

	bufferWriter.AppendChar('E');
	bufferWriter.AppendChar('N');
	bufferWriter.AppendChar('D');
	bufferWriter.AppendChar('T');
}

void ConvexSceneEditorTest::WriteOutChunk(BufferWriter& bufferWriter, uint8_t chunkType)
{
	GHCSChunk writtenChunk;
	writtenChunk.m_chunkType = chunkType;
	writtenChunk.m_chunkHeaderStart = (unsigned int)bufferWriter.GetAppendedSize();


	bufferWriter.AppendChar('G');
	bufferWriter.AppendChar('H');
	bufferWriter.AppendChar('C');
	bufferWriter.AppendChar('K');

	bufferWriter.AppendByte(chunkType);
	bufferWriter.AppendByte((uint8_t)bufferWriter.GetEndianMode());
	bufferWriter.AppendUint32(5);
	unsigned int chunkSizeStartOffset = (unsigned int)bufferWriter.GetAppendedSize();

	Vec2 screenBounds = m_game->m_screenCamera.GetOrthoDimensions();
	if (chunkType == 0x01)
	{
		//AABB2
		bufferWriter.AppendVec2(Vec2::ZERO);
		bufferWriter.AppendVec2(Vec2(screenBounds.x, screenBounds.y));
		bufferWriter.AppendUshort((unsigned short)m_sceneObjects.size());
	}
	else if (chunkType == 0x02)
	{
		//add convex poly data
		bufferWriter.AppendUshort((unsigned short)m_sceneObjects.size());
		for (int i = 0; i < (int)m_sceneObjects.size(); i++)
		{
			uint8_t verts = (uint8_t)m_sceneObjects[i].m_convexPoly.m_vertexPositions.size();
			bufferWriter.AppendByte(verts);
			for (int v = 0; v < (int)verts; v++)
			{
				bufferWriter.AppendVec2(m_sceneObjects[i].m_convexPoly.m_vertexPositions[v]);
			}
		}
	}
	else if (chunkType == 0x80)
	{
		bufferWriter.AppendUshort((uint16_t)m_sceneObjects.size());
		for (int i = 0; i < (int)m_sceneObjects.size(); i++)
		{
			uint8_t planes = (uint8_t)m_sceneObjects[i].m_convexHull.m_boundingPlanes.size();
			bufferWriter.AppendByte(planes);
			for (uint8_t p = 0; p < planes; p++)
			{
				bufferWriter.AppendVec2(m_sceneObjects[i].m_convexHull.m_boundingPlanes[p].m_normal);
				bufferWriter.AppendFloat(m_sceneObjects[i].m_convexHull.m_boundingPlanes[p].m_distFromOriginAlongNormal);
			}

		}
	}
	else if (chunkType == 0x81)
	{
		bufferWriter.AppendUshort((uint16_t)m_sceneObjects.size());
		for (int i = 0; i < (int)m_sceneObjects.size(); i++)
		{
			bufferWriter.AppendVec2(m_sceneObjects[i].m_boundingDisc.m_centerPosition);
			bufferWriter.AppendFloat(m_sceneObjects[i].m_boundingDisc.m_radius);
		}
	}
	else if (chunkType == 0x88)
	{
		bufferWriter.AppendVec2(Vec2::ZERO);
		bufferWriter.AppendVec2(Vec2(screenBounds.x, screenBounds.y));
		bufferWriter.AppendUshort((uint16_t)m_sceneObjects.size());
		for (int i = 0; i < (int)m_sceneObjects.size(); i++)
		{
			bufferWriter.AppendUint64(m_sceneObjects[i].m_sceneMask);
		}
	}
	unsigned int chunkSizeEndOffset = (unsigned int)bufferWriter.GetAppendedSize();
	unsigned int chunkSize = chunkSizeEndOffset - chunkSizeStartOffset;
	bufferWriter.WriteUint32AtOffset(chunkSizeStartOffset - 4, chunkSize);
	writtenChunk.m_chunkTotalSize = chunkSize;
	m_loadedChunks.push_back(writtenChunk);

	bufferWriter.AppendChar('E');
	bufferWriter.AppendChar('N');
	bufferWriter.AppendChar('D');
	bufferWriter.AppendChar('C');
}

void ConvexSceneEditorTest::LoadGHCS(BufferParser& bufferParser)
{
	char headerByte1 = bufferParser.ParseChar();
	char headerByte2 = bufferParser.ParseChar();
	char headerByte3 = bufferParser.ParseChar();
	char headerByte4 = bufferParser.ParseChar();
	uint8_t cohortByte = bufferParser.ParseByte();
	uint8_t versionMajor = bufferParser.ParseByte();
	uint8_t versionMinor = bufferParser.ParseByte();
	uint8_t enianMode = bufferParser.ParseByte();

	ASSERT_RECOVERABLE(headerByte1 == 'G', "Failed");
	ASSERT_RECOVERABLE(headerByte2 == 'H', "Failed");
	ASSERT_RECOVERABLE(headerByte3 == 'C', "Failed");
	ASSERT_RECOVERABLE(headerByte4 == 'S', "Failed");

	ASSERT_RECOVERABLE(cohortByte == 33, "Failed");
	ASSERT_RECOVERABLE(versionMajor == 1, "Incompatable version");
	ASSERT_RECOVERABLE(versionMinor == 1, "Incompatable version");
	bufferParser.SetEndianMode((BufferEndianMode)enianMode);

	unsigned int tocOffset = bufferParser.ParseUint32();

	char footerByte1 = bufferParser.ParseChar();
	char footerByte2 = bufferParser.ParseChar();
	char footerByte3 = bufferParser.ParseChar();
	char footerByte4 = bufferParser.ParseChar();
	ASSERT_RECOVERABLE(footerByte1 == 'E', "Failed");
	ASSERT_RECOVERABLE(footerByte2 == 'N', "Failed");
	ASSERT_RECOVERABLE(footerByte3 == 'D', "Failed");
	ASSERT_RECOVERABLE(footerByte4 == 'H', "Failed");

	bufferParser.SetCurrentBytePosition((int)tocOffset);
	char tocByte1 = bufferParser.ParseChar();
	char tocByte2 = bufferParser.ParseChar();
	char tocByte3 = bufferParser.ParseChar();
	char tocByte4 = bufferParser.ParseChar();

	ASSERT_RECOVERABLE(tocByte1 == 'G', "Failed");
	ASSERT_RECOVERABLE(tocByte2 == 'H', "Failed");
	ASSERT_RECOVERABLE(tocByte3 == 'T', "Failed");
	ASSERT_RECOVERABLE(tocByte4 == 'C', "Failed");
	uint8_t numberOfChunks = bufferParser.ParseByte();
	bool boundingDiscChunkPresent = false;
	bool bitBucketChunkPresent = false;
	bool convexHullChunkPresent = false;
	uint32_t tocChunksBeginOffset = bufferParser.GetCurrentBytePosition();

	//check in advance to see if bounding disc and bit bucket chunks are present
	for (uint8_t i = 0; i < numberOfChunks; i++)
	{
		uint8_t chunkType = bufferParser.ParseByte();
		if (chunkType == 0x80)
		{
			convexHullChunkPresent = true;
		}
		if (chunkType == 0x81)
		{
			boundingDiscChunkPresent = true;
		}
		if (chunkType == 0x88)
		{
			bitBucketChunkPresent = true;
		}
		bufferParser.SetCurrentBytePosition(bufferParser.GetCurrentBytePosition() + 8);
	}

	bufferParser.SetCurrentBytePosition(tocChunksBeginOffset);
	for (uint8_t i = 0; i < numberOfChunks; i++)
	{
		LoadChunk(bufferParser, boundingDiscChunkPresent, bitBucketChunkPresent, convexHullChunkPresent);
	}

}

void ConvexSceneEditorTest::LoadChunk(BufferParser& bufferParser, bool boundingDiscChunkPresent, bool bitBucketChunkPresent, bool convexHullChunkPresent)
{
	uint8_t chunkType = bufferParser.ParseByte();
	uint32_t chunkStart = bufferParser.ParseUint32();
	uint32_t chunkSizeInTOC = bufferParser.ParseUint32();

	//think it is absolute offset instead of relative but not 100% sure
	int returnToHeaderOffset = bufferParser.GetCurrentBytePosition();

	//scene info
	if (chunkType == 0x01)
	{
		bufferParser.SetCurrentBytePosition(chunkStart);
		char chunkByte1 = bufferParser.ParseChar();
		char chunkByte2 = bufferParser.ParseChar();
		char chunkByte3 = bufferParser.ParseChar();
		char chunkByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkByte1 == 'G', "Failed");
		ASSERT_RECOVERABLE(chunkByte2 == 'H', "Failed");
		ASSERT_RECOVERABLE(chunkByte3 == 'C', "Failed");
		ASSERT_RECOVERABLE(chunkByte4 == 'K', "Failed");

		uint8_t chunkByte = bufferParser.ParseByte();
		ASSERT_RECOVERABLE(chunkByte == 0x01, "Wrong chunk");

		uint8_t endianType = bufferParser.ParseByte();
		bufferParser.SetEndianMode((BufferEndianMode)endianType);
		uint32_t chunkPayloadSize = bufferParser.ParseUint32();
		ASSERT_RECOVERABLE(chunkPayloadSize == 18, "wrong size for chunk 1");
		Vec2 worldMins = bufferParser.ParseVec2();
		Vec2 worldMaxs = bufferParser.ParseVec2();

		AABB2 worldBounds(worldMins, worldMaxs);
		m_game->m_screenCamera.SetOrthographicView(worldMins, worldMaxs);
		m_numObjects = (int)bufferParser.ParseUshort();

		char chunkEndByte1 = bufferParser.ParseChar();
		char chunkEndByte2 = bufferParser.ParseChar();
		char chunkEndByte3 = bufferParser.ParseChar();
		char chunkEndByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkEndByte1 == 'E', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte2 == 'N', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte3 == 'D', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte4 == 'C', "Failed");

	}
	else if (chunkType == 0x02)
	{
		bufferParser.SetCurrentBytePosition(chunkStart);
		char chunkByte1 = bufferParser.ParseChar();
		char chunkByte2 = bufferParser.ParseChar();
		char chunkByte3 = bufferParser.ParseChar();
		char chunkByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkByte1 == 'G', "Failed");
		ASSERT_RECOVERABLE(chunkByte2 == 'H', "Failed");
		ASSERT_RECOVERABLE(chunkByte3 == 'C', "Failed");
		ASSERT_RECOVERABLE(chunkByte4 == 'K', "Failed");

		uint8_t chunkByte = bufferParser.ParseByte();
		ASSERT_RECOVERABLE(chunkByte == 2, "Wrong chunk");

		uint8_t endianType = bufferParser.ParseByte();
		bufferParser.SetEndianMode((BufferEndianMode)endianType);
		uint32_t chunkPayloadSize = bufferParser.ParseUint32();
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkSizeInTOC - 14, "Failed");
		uint32_t chunkDataStart = bufferParser.GetCurrentBytePosition();
		m_sceneObjects.clear();

		uint16_t numberOfObjects = bufferParser.ParseUshort();
		for (uint16_t i = 0; i < numberOfObjects; i++)
		{
			uint8_t numVerts = bufferParser.ParseByte();
			std::vector<Vec2> verts;
			verts.reserve((size_t)numVerts);

			for (uint8_t v = 0; v < numVerts; v++)
			{
				verts.push_back(bufferParser.ParseVec2());
			}
			AddConvexObjectFromVerts(verts, boundingDiscChunkPresent, bitBucketChunkPresent, convexHullChunkPresent);
		}
		ASSERT_RECOVERABLE((size_t)numberOfObjects == m_sceneObjects.size(), "Messed up object parsing");
		uint32_t chunkCalculatedSize= (uint32_t)bufferParser.GetCurrentBytePosition() - chunkDataStart;
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkCalculatedSize, "Messed up");

		char chunkEndByte1 = bufferParser.ParseChar();
		char chunkEndByte2 = bufferParser.ParseChar();
		char chunkEndByte3 = bufferParser.ParseChar();
		char chunkEndByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkEndByte1 == 'E', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte2 == 'N', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte3 == 'D', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte4 == 'C', "Failed");
	}
	else if (chunkType == 0x80)
	{
		bufferParser.SetCurrentBytePosition(chunkStart);
		char chunkByte1 = bufferParser.ParseChar();
		char chunkByte2 = bufferParser.ParseChar();
		char chunkByte3 = bufferParser.ParseChar();
		char chunkByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkByte1 == 'G', "Failed");
		ASSERT_RECOVERABLE(chunkByte2 == 'H', "Failed");
		ASSERT_RECOVERABLE(chunkByte3 == 'C', "Failed");
		ASSERT_RECOVERABLE(chunkByte4 == 'K', "Failed");

		uint8_t chunkByte = bufferParser.ParseByte();
		ASSERT_RECOVERABLE(chunkByte == 0x80, "Wrong chunk");

		uint8_t endianType = bufferParser.ParseByte();
		bufferParser.SetEndianMode((BufferEndianMode)endianType);
		uint32_t chunkPayloadSize = bufferParser.ParseUint32();
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkSizeInTOC - 14, "Failed");
		uint32_t chunkDataStart = bufferParser.GetCurrentBytePosition();

		uint16_t numObjectsInScene = bufferParser.ParseUshort();
		ASSERT_RECOVERABLE((size_t)numObjectsInScene == m_sceneObjects.size(), "Inconsistency!!!");

		for (int i = 0; i < (int)m_sceneObjects.size(); i++)
		{
			uint8_t planes = bufferParser.ParseByte();
			for (uint8_t p = 0; p < planes; p++)
			{
				Vec2 planeNormal = bufferParser.ParseVec2();
				float planeFloat = bufferParser.ParseFloat();
				Plane2 currentPlane(planeNormal, planeFloat);
				m_sceneObjects[i].m_convexHull.m_boundingPlanes.push_back(currentPlane);
			}
		}

		uint32_t chunkCalculatedSize = (uint32_t)bufferParser.GetCurrentBytePosition() - chunkDataStart;
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkCalculatedSize, "Messed up");

		char chunkEndByte1 = bufferParser.ParseChar();
		char chunkEndByte2 = bufferParser.ParseChar();
		char chunkEndByte3 = bufferParser.ParseChar();
		char chunkEndByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkEndByte1 == 'E', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte2 == 'N', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte3 == 'D', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte4 == 'C', "Failed");
	}

	//bounding disc chunk
	else if (chunkType == 0x81)
	{
		bufferParser.SetCurrentBytePosition(chunkStart);
		char chunkByte1 = bufferParser.ParseChar();
		char chunkByte2 = bufferParser.ParseChar();
		char chunkByte3 = bufferParser.ParseChar();
		char chunkByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkByte1 == 'G', "Failed");
		ASSERT_RECOVERABLE(chunkByte2 == 'H', "Failed");
		ASSERT_RECOVERABLE(chunkByte3 == 'C', "Failed");
		ASSERT_RECOVERABLE(chunkByte4 == 'K', "Failed");

		uint8_t chunkByte = bufferParser.ParseByte();
		ASSERT_RECOVERABLE(chunkByte == 0x81, "Wrong chunk");

		uint8_t endianType = bufferParser.ParseByte();
		bufferParser.SetEndianMode((BufferEndianMode)endianType);
		uint32_t chunkPayloadSize = bufferParser.ParseUint32();
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkSizeInTOC - 14, "Failed");
		uint32_t chunkDataStart = bufferParser.GetCurrentBytePosition();

		uint16_t numObjectsInScene = bufferParser.ParseUshort();
		ASSERT_RECOVERABLE((size_t)numObjectsInScene == m_sceneObjects.size(), "Inconsistency!!!");

		for (int i = 0; i < (int)m_sceneObjects.size(); i++)
		{
			m_sceneObjects[i].m_boundingDisc.m_centerPosition = bufferParser.ParseVec2();
			m_sceneObjects[i].m_boundingDisc.m_radius = bufferParser.ParseFloat();
		}

		uint32_t chunkCalculatedSize = (uint32_t)bufferParser.GetCurrentBytePosition() - chunkDataStart;
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkCalculatedSize, "Messed up");

		char chunkEndByte1 = bufferParser.ParseChar();
		char chunkEndByte2 = bufferParser.ParseChar();
		char chunkEndByte3 = bufferParser.ParseChar();
		char chunkEndByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkEndByte1 == 'E', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte2 == 'N', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte3 == 'D', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte4 == 'C', "Failed");
	}
	//bit bucket
	else if (chunkType == 0x88)
	{
		bufferParser.SetCurrentBytePosition(chunkStart);
		char chunkByte1 = bufferParser.ParseChar();
		char chunkByte2 = bufferParser.ParseChar();
		char chunkByte3 = bufferParser.ParseChar();
		char chunkByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkByte1 == 'G', "Failed");
		ASSERT_RECOVERABLE(chunkByte2 == 'H', "Failed");
		ASSERT_RECOVERABLE(chunkByte3 == 'C', "Failed");
		ASSERT_RECOVERABLE(chunkByte4 == 'K', "Failed");

		uint8_t chunkByte = bufferParser.ParseByte();
		ASSERT_RECOVERABLE(chunkByte == 0x88, "Wrong chunk");

		uint8_t endianType = bufferParser.ParseByte();
		bufferParser.SetEndianMode((BufferEndianMode)endianType);
		uint32_t chunkPayloadSize = bufferParser.ParseUint32();
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkSizeInTOC - 14, "Failed");
		uint32_t chunkDataStart = bufferParser.GetCurrentBytePosition();

		Vec2 sceneMins = bufferParser.ParseVec2();
		Vec2 sceneMaxs = bufferParser.ParseVec2();

		Vec2 actualSceneMins = m_game->m_screenCamera.GetOrthoBottomLeft();
		Vec2 actualSceneMaxs = m_game->m_screenCamera.GetOrthoTopRight();

		if (fabsf(sceneMins.x - actualSceneMins.x) > .0001f || fabsf(sceneMins.y - actualSceneMins.y) > .0001f
			|| fabsf(sceneMaxs.x - actualSceneMaxs.x) > .0001f || fabsf(sceneMaxs.y - actualSceneMaxs.y) > .0001f)
		{
			ERROR_RECOVERABLE("SCENE BOUNDS NOT RIGHT");
		}

		uint16_t numObjectsInScene = bufferParser.ParseUshort();
		ASSERT_RECOVERABLE((size_t)numObjectsInScene == m_sceneObjects.size(), "Inconsistency!!!");

		for (int i = 0; i < (int)m_sceneObjects.size(); i++)
		{
			m_sceneObjects[i].m_sceneMask = bufferParser.ParseUint64();
		}

		uint32_t chunkCalculatedSize = (uint32_t)bufferParser.GetCurrentBytePosition() - chunkDataStart;
		ASSERT_RECOVERABLE(chunkPayloadSize == chunkCalculatedSize, "Messed up");

		char chunkEndByte1 = bufferParser.ParseChar();
		char chunkEndByte2 = bufferParser.ParseChar();
		char chunkEndByte3 = bufferParser.ParseChar();
		char chunkEndByte4 = bufferParser.ParseChar();

		ASSERT_RECOVERABLE(chunkEndByte1 == 'E', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte2 == 'N', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte3 == 'D', "Failed");
		ASSERT_RECOVERABLE(chunkEndByte4 == 'C', "Failed");
	}
	bufferParser.SetCurrentBytePosition(returnToHeaderOffset);

}

bool ConvexSceneEditorTest::Event_SaveConvexScene(EventArgs& args)
{
	std::string fileName = args.GetValue("name", "MissingName");
	if (fileName == "MissingName")
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "missing name argument for SaveConvexScene");
		return false;
	}
	std::string filePath = "Data/Scenes/" + fileName + ".ghcs";
	VisualTest* currentVisualTest = g_theApp->GetGame()->GetCurrentVisualTest();
	ConvexSceneEditorTest* thisTest = dynamic_cast<ConvexSceneEditorTest*>(currentVisualTest);
	thisTest->m_sceneData.clear();
	BufferWriter sceneWriter(thisTest->m_sceneData);

	bool skipDisc = args.GetValue("skipDisc", false);
	bool skipConvexHull = args.GetValue("skipConvexHull", false);
	bool skipBitBuckets = args.GetValue("skipBitBuckets", false);

	if (thisTest != nullptr)
	{
		thisTest->WriteOutGHCS(sceneWriter, skipDisc, skipConvexHull, skipBitBuckets);
		if (thisTest->m_sceneData.size() > 0)
		{
			bool success = WriteBufferToFile(thisTest->m_sceneData, filePath);
			if (success)
			{
				g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Successfully wrote data to %s", filePath.c_str()));
			}
			else
			{
				g_theDevConsole->AddLine(DevConsole::INFO_ERROR, Stringf("Error writing data to %s", filePath.c_str()));
			}
		}
	}
	return false;
}

bool ConvexSceneEditorTest::Event_LoadConvexScene(EventArgs& args)
{
	std::string fileName = args.GetValue("name", "MissingName");
	if (fileName == "MissingName")
	{
		g_theDevConsole->AddLine(DevConsole::INFO_ERROR, "missing name argument for LoadConvexScene");
		return false;
	}
	VisualTest* currentVisualTest = g_theApp->GetGame()->GetCurrentVisualTest();
	ConvexSceneEditorTest* thisTest = dynamic_cast<ConvexSceneEditorTest*>(currentVisualTest);
	std::string filePath = "Data/Scenes/" + fileName + ".ghcs";
	FileReadToBuffer(thisTest->m_sceneData, filePath);
	
	BufferParser sceneParser(thisTest->m_sceneData);
	if (thisTest != nullptr)
	{
		thisTest->LoadGHCS(sceneParser);
		if (thisTest->m_sceneData.size() > 0)
		{
			bool success = WriteBufferToFile(thisTest->m_sceneData, filePath);
			if (success)
			{
				g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, Stringf("Successfully loaded data from %s", filePath.c_str()));
			}
			else
			{
				g_theDevConsole->AddLine(DevConsole::INFO_ERROR, Stringf("Error loading data from %s", filePath.c_str()));

			}
		}
	}
	return false;
}

void ConvexSceneEditorTest::AddConvexObjectFromVerts(std::vector<Vec2> const& verts, bool boundingDiscChunkPresent, bool bitBucketChunkPresent, bool convexHullCunkPresent)
{
	BoundingDisc2D boundingDisc;

	if (!boundingDiscChunkPresent)
	{
		Vec2 center = Vec2::ZERO;
		for (int i = 0; i < (int)verts.size(); i++)
		{
			center += verts[i];
		}
		center /= (float)verts.size();


		float radius = 0.f;
		for (int i = 0; i < (int)verts.size(); i++)
		{
			float currDistance = GetDistance2D(center, verts[i]);
			if (currDistance > radius)
			{
				radius = currDistance;
			}
		}
		boundingDisc.m_centerPosition = center;
		boundingDisc.m_radius = radius;
	}

	ConvexPolySceneObject sceneObject = ConvexPolySceneObject(verts, boundingDisc, !convexHullCunkPresent);
	if (!bitBucketChunkPresent)
	{
		CreateSceneObjectMask(sceneObject);
	}
	m_sceneObjects.push_back(sceneObject);
}

void ConvexSceneEditorTest::AddVertsForGridLines(std::vector<Vertex_PCU>& verts)
{
	Vec2 screenBounds = m_game->m_screenCamera.GetOrthoDimensions();
	float lineXOffset = screenBounds.x / m_gridWidth;
	float lineYOffset = screenBounds.y / m_gridHeight;
	for (int y = 1; y < m_gridWidth; y++)
	{
		AddVertsForLine2D(verts, Vec2(-100, (float)y * lineYOffset), Vec2(screenBounds.x * 2, (float)y * lineYOffset), .25f, Rgba8::BLUE);
	}
	for (int x = 1; x < m_gridWidth; x++)
	{
		AddVertsForLine2D(verts, Vec2((float)x * lineXOffset, -100.f), Vec2((float)x * lineXOffset, screenBounds.y * 2.f), .25f, Rgba8::BLUE);
	}
}

void ConvexSceneEditorTest::AddVertsForBoundingDiscs(std::vector<Vertex_PCU>& verts)
{
	for (int i = 0; i < (int)m_sceneObjects.size(); i++)
	{
		AddVertsForRing2D(verts, m_sceneObjects[i].m_boundingDisc.m_centerPosition, m_sceneObjects[i].m_boundingDisc.m_radius, .1f, Rgba8::WHITE);
	}
}

ConvexPolySceneObject::ConvexPolySceneObject(std::vector<Vec2> vertexes, BoundingDisc2D boundingDisc, bool loadConvexHull)
	: m_convexPoly(ConvexPoly2D(vertexes))
	, m_boundingDisc(boundingDisc)
{
	if (loadConvexHull)
	{
		m_convexHull = ConvexHull2D(vertexes);
	}
}

void ConvexPolySceneObject::AddVertsForConvexPoly(std::vector<Vertex_PCU>& verts, Rgba8 color)
{
	Vec2 centerPos;
	for (int i = 0; i < (int)m_convexPoly.m_vertexPositions.size(); i++)
	{
		centerPos += m_convexPoly.m_vertexPositions[i];
	}
	centerPos /= (float)m_convexPoly.m_vertexPositions.size();

	for (int i = 0; i < (int)m_convexPoly.m_vertexPositions.size() - 1; i++)
	{
		verts.push_back(Vertex_PCU(centerPos.GetXYZ(), color));
		verts.push_back(Vertex_PCU(m_convexPoly.m_vertexPositions[i].GetXYZ(), color));
		verts.push_back(Vertex_PCU(m_convexPoly.m_vertexPositions[i + 1].GetXYZ(), color));
	}
	verts.push_back(Vertex_PCU(centerPos.GetXYZ(), color));
	verts.push_back(Vertex_PCU(m_convexPoly.m_vertexPositions[m_convexPoly.m_vertexPositions.size() - 1].GetXYZ(), color));
	verts.push_back(Vertex_PCU(m_convexPoly.m_vertexPositions[0].GetXYZ(), color));
}

void ConvexPolySceneObject::AddVertsForConvexPolyEdges(std::vector<Vertex_PCU>& verts, float lineThickness, Rgba8 color)
{
	for (int i = 0; i < (int)m_convexPoly.m_vertexPositions.size() - 1; i++)
	{
		AddVertsForLine2D(verts, m_convexPoly.m_vertexPositions[i], m_convexPoly.m_vertexPositions[i + 1], lineThickness, color);
	}
	AddVertsForLine2D(verts, m_convexPoly.m_vertexPositions[m_convexPoly.m_vertexPositions.size() - 1], m_convexPoly.m_vertexPositions[0], lineThickness, color);
	
}