#include "VisualTest.hpp"


class A5VisualTest : public VisualTest
{
public:
	A5VisualTest(VisualTestType myTestType, Game* game);
	~A5VisualTest() = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void InitializeTest() override;
	void RandomizeTest() override;
	float m_pointMoveSpeed = 25.f;
	Vec2 m_point;

	AABB2 m_aabb2;

	OBB2 m_obb2;

	Vec2 m_discPos;
	float m_discRadius;

	Vec2 m_capsuleBoneStart;
	Vec2 m_capsuleBoneEnd;
	float m_capsuleRadius;

	Vec2 m_lineSegmentStart;
	Vec2 m_lineSegmentEnd;

	Vec2 m_infiniteLineSegmentDir;
	Vec2 m_infiniteLineSegmentPos;
	std::vector<Vertex_PCU> m_textVerts;
};