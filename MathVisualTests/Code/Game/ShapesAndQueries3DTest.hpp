#pragma once
#include "Game/VisualTest.hpp"
#include "Game/PhysicsShape3D.hpp"

class Player;

constexpr float RAYCAST_LENGTH = 5.f;

class ShapesAndQueries3DTest : public VisualTest
{
public:
	ShapesAndQueries3DTest(VisualTestType myTestType, Game* game);
	void Update(float deltaSeconds) override;
	void Render();
	void InitializeTest() override;
	void RandomizeTest() override;
	RaycastResult3D m_closestRaycastHit;
	VisualTestType m_visualTestType;
	std::vector<Vertex_PCU> m_textVerts;
	std::vector<PhysicsShape3D*> m_allPhysicsShapes;
	std::vector<std::vector<PhysicsShape3D*>> m_physicsShapesByType;
	Player* m_player = nullptr;
	bool m_firstFrame = true;
	Texture* m_testTexture = nullptr;
	PhysicsShape3D* m_grabbedShape = nullptr;
	PhysicsShape3D* m_hitShape = nullptr;
	bool m_lockRaycast = false;
	Vec3 m_lockedRaycastStartPos = Vec3::ZERO;
	Vec3 m_lockedRaycastDir = Vec3::ZERO;

private:
	void AddPhysicsSphere3D();
	void AddPhysicsCylinder3D();
	void AddPhysicsAABB3D();
	void AddPhysicsOBB3D();
	void AddPhysicsPlane3D();
	void HandleMouseMode();
	void GetGrabbedShape();
	void HandleGrabbing();
	void HandleThirdPersonMode();
	void UpdatePlayerAndGrabbedShape(float deltaSeconds);
	void UpdatePhysicsShapeQueries();
	void AddCompass();

};