#include "Game/ShapesAndQueries3DTest.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/PhysicsSphere3D.hpp"
#include "Game/PhysicsCylinder3D.hpp"
#include "Game/PhysicsAABB3D.hpp"
#include "Game/PhysicsPlane3D.hpp"
#include "Game/PhysicsOBB3D.hpp"

ShapesAndQueries3DTest::ShapesAndQueries3DTest(VisualTestType myTestType, Game* game)
	: VisualTest(myTestType, game)
{
	m_player = new Player(m_game, Vec3(-2.f, 0.f, 0.f));
	m_player->m_orientationDegrees = EulerAngles(0.f, 0.f, 0.f);

	m_player->m_playerCamera.SetPerspectiveView(g_theWindow->GetConfig().m_clientAspect, 60.f, .1f, 100.f);
	m_player->m_playerCamera.m_mode = Camera::eMode_Perspective;
	m_player->m_playerCamera.SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.f, 1.f, 0.f));
	m_player->m_playerCamera.m_position = m_player->m_position;
	m_player->m_playerCamera.m_orientation = m_player->m_orientationDegrees;
}

void ShapesAndQueries3DTest::InitializeTest()
{
	m_textVerts.clear();
	g_bitmapFont->AddVertsForText2D(m_textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .95f), 1.75f, "(Mode F6/F7 for prev/next): 3D Shapes and Queries", Rgba8(255, 250, 100));
	g_bitmapFont->AddVertsForText2D(m_textVerts, Vec2(GetScreenWidth() * .025f, GetScreenHeight() * .9f), 1.75f,
		"F8 to randomize; WASD = fly horizontal; Z/C = fly vertical; space = lock raycast; hold T = slow", Rgba8(100, 255, 200));
	m_player->m_position = Vec3(-2.f, 0.f, 0.f);
	m_player->m_orientationDegrees = EulerAngles(0.f, 0.f, 0.f);
	g_theInput->SetCursorMode(true, true);
	DebugRenderClear();
	DebugAddWorldBasis(Mat44(), -1.f, DebugRenderMode::USE_DEPTH);
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");

	RandomizeTest();
}

void ShapesAndQueries3DTest::Update(float deltaSeconds)
{
	HandleMouseMode();
	UpdatePlayerAndGrabbedShape(deltaSeconds);
	HandleThirdPersonMode();
	UpdatePhysicsShapeQueries();
	HandleGrabbing();
	AddCompass();
}

void ShapesAndQueries3DTest::HandleGrabbing()
{
	//handle grab input
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		m_grabbedShape = m_hitShape;
		if (m_grabbedShape)
		{
			m_grabbedShape->m_color = m_grabbedShape->c_grabbedColor;
		}
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE) && m_grabbedShape != nullptr)
	{
		if (m_grabbedShape->m_texture)
		{
			m_grabbedShape->m_color = Rgba8::WHITE;
		}
		else
		{
			m_grabbedShape->m_color = m_grabbedShape->c_wireframeDefaultColor;
		}
		m_grabbedShape = nullptr;
	}
}

void ShapesAndQueries3DTest::HandleThirdPersonMode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		m_lockRaycast = !m_lockRaycast;

		if (m_lockRaycast)
		{
			m_lockedRaycastStartPos = m_player->m_position;
			m_lockedRaycastDir = m_player->m_orientationDegrees.GetIFwd();
		}
		else
		{
			m_lockedRaycastStartPos = Vec3::ZERO;
			m_lockedRaycastDir = Vec3::ZERO;
		}

	}
}

void ShapesAndQueries3DTest::UpdatePlayerAndGrabbedShape(float deltaSeconds)
{
	//update player and shape queries
	Vec3 prevPlayerPosition = m_player->m_position;
	m_player->Update(deltaSeconds);

	//move grabbed shape
	if (m_grabbedShape)
	{
		m_grabbedShape->SetPosition(m_grabbedShape->GetPosition() + (m_player->m_position - prevPlayerPosition));
	}
}

void ShapesAndQueries3DTest::UpdatePhysicsShapeQueries()
{
	//reset color of previous hit shape
	if (m_hitShape != nullptr && m_hitShape != m_grabbedShape)
	{
		if (m_hitShape->m_texture != nullptr)
		{
			m_hitShape->m_color = Rgba8::WHITE;
		}
		else
		{
			m_hitShape->m_color = m_hitShape->c_wireframeDefaultColor;
		}
	}
	m_hitShape = nullptr;

	m_closestRaycastHit = RaycastResult3D();
	Vec3 raycastStartPos = m_player->m_position;
	Vec3 raycastDir = m_player->m_orientationDegrees.GetIFwd();
	if (m_lockRaycast)
	{
		raycastStartPos = m_lockedRaycastStartPos;
		raycastDir = m_lockedRaycastDir;
	}
	for (size_t i = 0; i < m_allPhysicsShapes.size(); i++)
	{
		m_allPhysicsShapes[i]->SetNearestPoint(raycastStartPos);
		m_allPhysicsShapes[i]->CheckForOverlapWithOtherShapes(m_physicsShapesByType);
		RaycastResult3D currRaycastResult = m_allPhysicsShapes[i]->RaycastVsShape(raycastStartPos, raycastDir, 5.f);
		if (currRaycastResult.m_didImpact && (!m_closestRaycastHit.m_didImpact || currRaycastResult.m_impactDist < m_closestRaycastHit.m_impactDist))
		{
			m_hitShape = m_allPhysicsShapes[i];
			m_closestRaycastHit = currRaycastResult;
		}
	}
	if (m_hitShape != nullptr && m_hitShape != m_grabbedShape)
	{
		m_hitShape->m_color = m_hitShape->c_hitColor;
	}
}

void ShapesAndQueries3DTest::AddCompass()
{
	//compass
	Mat44 compassTransform;
	compassTransform.SetTranslation3D(m_player->m_position + m_player->m_orientationDegrees.GetIFwd() * .2f);
	compassTransform.AppendScaleUniform3D(.01f);
	DebugAddWorldBasis(compassTransform, 0.f, DebugRenderMode::ALWAYS);
}

void ShapesAndQueries3DTest::Render()
{
	//render physics shapes
	g_theRenderer->BeginCamera(m_player->m_playerCamera);
	for (size_t i = 0; i < m_allPhysicsShapes.size(); i++)
	{
		m_allPhysicsShapes[i]->Render();
	}
	g_theRenderer->EndCamera(m_player->m_playerCamera);

	//render debug objects
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);

	constexpr float arrowThickness = .01f;
	constexpr float pointThickness = .02f;
	//render raycast
	if (m_closestRaycastHit.m_didImpact)
	{
		DebugAddWorldPoint(m_closestRaycastHit.m_impactPos, pointThickness, 0.f);
		DebugAddWorldArrow(m_closestRaycastHit.m_impactPos, m_closestRaycastHit.m_impactPos + m_closestRaycastHit.m_impactNormal, arrowThickness, 0.f, Rgba8::YELLOW);
		if (m_lockRaycast)
		{
			DebugAddWorldArrow(m_lockedRaycastStartPos, m_closestRaycastHit.m_impactPos, arrowThickness, 0.f, Rgba8::RED);
			DebugAddWorldArrow(m_closestRaycastHit.m_impactPos, m_lockedRaycastStartPos + m_lockedRaycastDir * RAYCAST_LENGTH, arrowThickness, 0.f, Rgba8(150, 150, 150));
		}
	}
	else
	{
		if (m_lockRaycast)
		{
			DebugAddWorldPoint(m_lockedRaycastStartPos, pointThickness, 0.f);
			DebugAddWorldArrow(m_lockedRaycastStartPos, m_lockedRaycastStartPos + m_lockedRaycastDir * RAYCAST_LENGTH, arrowThickness, 0.f, Rgba8::GREEN);
		}
	}
	DebugRenderWorld(m_player->m_playerCamera);
	
	//render text
	g_theRenderer->BeginCamera(m_game->m_screenCamera);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(g_bitmapFont->GetTexture());
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizeMode(RasterizeMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexArray(m_textVerts.size(), m_textVerts.data());

	g_theRenderer->EndCamera(m_game->m_screenCamera);
}

void ShapesAndQueries3DTest::RandomizeTest()
{

	//clear all shapes
	m_allPhysicsShapes.clear();
	if (m_physicsShapesByType.size() > 0)
	{
		for (size_t i = 0; i < (size_t)ShapeType::COUNT; i++)
		{
			m_physicsShapesByType[i].clear();
		}
	}

	m_physicsShapesByType.resize((size_t)ShapeType::COUNT);

	//Add Spheres
	for (size_t i = 0; i < 4; i++)
	{
		AddPhysicsSphere3D();
		AddPhysicsCylinder3D();
		AddPhysicsAABB3D();
		AddPhysicsOBB3D();
	}
	AddPhysicsPlane3D();
}

void ShapesAndQueries3DTest::AddPhysicsSphere3D()
{
	bool drawWireFrame = g_randGen->RollRandomFloatZeroToOne() > .5f;
	Texture* sphereTexture = nullptr;
	if (!drawWireFrame)
	{
		sphereTexture = m_testTexture;
	}
	Vec3 spherePosition(g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f));
	float sphereRadius = g_randGen->RollRandomFloatInRange(.5f, 1.5f);
	PhysicsSphere3D* sphere = new PhysicsSphere3D(spherePosition, ShapeType::SPHERE, drawWireFrame, sphereTexture, sphereRadius);
	m_allPhysicsShapes.push_back(sphere);
	m_physicsShapesByType[(int)ShapeType::SPHERE].push_back(sphere);
}

void ShapesAndQueries3DTest::AddPhysicsCylinder3D()
{
	bool drawWireFrame = g_randGen->RollRandomFloatZeroToOne() > .5f;
	Texture* sphereTexture = nullptr;
	if (!drawWireFrame)
	{
		sphereTexture = m_testTexture;
	}
	Vec3 cylinderPosition(g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f));
	float cylinderRadius = g_randGen->RollRandomFloatInRange(.5f, 1.5f);
	float cylinderHeight = g_randGen->RollRandomFloatInRange(1.f, 3.f);
	PhysicsCylinder3D* cylinder = new PhysicsCylinder3D(cylinderPosition, ShapeType::CYLINDER, drawWireFrame, sphereTexture, cylinderRadius, cylinderHeight);
	m_allPhysicsShapes.push_back(cylinder);
	m_physicsShapesByType[(int)ShapeType::CYLINDER].push_back(cylinder);
}

void ShapesAndQueries3DTest::AddPhysicsAABB3D()
{
	bool drawWireFrame = g_randGen->RollRandomFloatZeroToOne() > .5f;
	Texture* aabbTexture = nullptr;
	if (!drawWireFrame)
	{
		aabbTexture = m_testTexture;
	}
	Vec3 aabbPosition(g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f));
	Vec3 aabbDimensions(g_randGen->RollRandomFloatInRange(.5f, 2.f), g_randGen->RollRandomFloatInRange(.5f, 2.f), g_randGen->RollRandomFloatInRange(.5f, 2.f));
	PhysicsAABB3D* aabb = new PhysicsAABB3D(aabbPosition, ShapeType::AABB3, drawWireFrame, aabbTexture, aabbDimensions);
	m_allPhysicsShapes.push_back(aabb);
	m_physicsShapesByType[(int)ShapeType::AABB3].push_back(aabb);
}

void ShapesAndQueries3DTest::AddPhysicsOBB3D()
{
	bool drawWireFrame = g_randGen->RollRandomFloatZeroToOne() > .5f;
	Texture* obbTexture = nullptr;
	if (!drawWireFrame)
	{
		obbTexture = m_testTexture;
	}
	Vec3 obbPosition(g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f), g_randGen->RollRandomFloatInRange(-7.5f, 7.5f));
	Vec3 obbDimensions(g_randGen->RollRandomFloatInRange(.5f, 2.f), g_randGen->RollRandomFloatInRange(.5f, 2.f), g_randGen->RollRandomFloatInRange(.5f, 2.f));

	Vec3 iBasis = g_randGen->RollRandomNormalizedVec3();
	Vec3 jBasis = CrossProduct3D(iBasis, Vec3(0.f, 0.f, 1.f));
	Vec3 kBasis = CrossProduct3D(iBasis, jBasis);

	PhysicsOBB3D* obb = new PhysicsOBB3D(obbPosition, ShapeType::AABB3, drawWireFrame, obbTexture, iBasis, jBasis, kBasis, obbDimensions*.5f);
	m_allPhysicsShapes.push_back(obb);
	m_physicsShapesByType[(int)ShapeType::OBB3].push_back(obb);
}

void ShapesAndQueries3DTest::AddPhysicsPlane3D()
{
	bool drawWireFrame = g_randGen->RollRandomFloatZeroToOne() > .5f;
	Texture* planeTexture = nullptr;
	if (!drawWireFrame)
	{
		planeTexture = m_testTexture;
	}
	Plane3 plane;
	plane.m_normal = g_randGen->RollRandomNormalizedVec3();
	plane.m_distFromOriginAlongNormal = g_randGen->RollRandomFloatInRange(-10.f, 10.f);

	PhysicsPlane3D* physicsPlane = new PhysicsPlane3D(plane, ShapeType::PLANE, false, nullptr);
	m_allPhysicsShapes.push_back(physicsPlane);
	m_physicsShapesByType[(int)ShapeType::PLANE].push_back(physicsPlane);
}

void ShapesAndQueries3DTest::HandleMouseMode()
{
	if (!g_theWindow->IsWindowFocus())
	{
		g_theInput->SetCursorMode(false, false);
	}
	else
	{
		g_theInput->SetCursorMode(true, true);
	}
	//hacky fix to client delta being huge on first frame 
	if (m_firstFrame)
	{
		g_theInput->ResetCursorClientDelta();
		m_firstFrame = false;
	}
}

void ShapesAndQueries3DTest::GetGrabbedShape()
{
	m_grabbedShape = m_allPhysicsShapes[0];
	float currGrabbedDistance = GetDistance3D(m_player->m_position, m_grabbedShape->GetPosition());
	for (size_t i = 1; i < m_allPhysicsShapes.size(); i++)
	{
		float newShapeDistance = GetDistance3D(m_player->m_position, m_allPhysicsShapes[i]->GetPosition());
		if (newShapeDistance < currGrabbedDistance)
		{
			currGrabbedDistance = newShapeDistance;
			m_grabbedShape = m_allPhysicsShapes[i];
		}
	}
}
