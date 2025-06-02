#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

void DebugDrawLine(Vec2 const& pointA, Vec2 const& pointB, float thickness, Rgba8 const& color)
{
	Vec2 forwardDir = (pointB - pointA).GetNormalized();
	Vec2 rightDir = forwardDir.GetRotated90Degrees();
	Vec2 bottomRight = pointA - (forwardDir * thickness) + (rightDir * thickness);
	Vec2 bottomLeft = pointA - (forwardDir * thickness) - (rightDir * thickness);
	Vec2 topRight = pointB + (forwardDir * thickness) + (rightDir * thickness);
	Vec2 topLeft = pointB + (forwardDir * thickness) - (rightDir * thickness);
	Vertex_PCU tempWorldVerts[6];
	tempWorldVerts[0] = Vertex_PCU(Vec3(bottomLeft.x, bottomLeft.y, 0), color);
	tempWorldVerts[1] = Vertex_PCU(Vec3(bottomRight.x, bottomRight.y, 0), color);
	tempWorldVerts[2] = Vertex_PCU(Vec3(topRight.x, topRight.y, 0), color);

	tempWorldVerts[3] = Vertex_PCU(Vec3(topRight.x, topRight.y, 0), color);
	tempWorldVerts[4] = Vertex_PCU(Vec3(topLeft.x, topLeft.y, 0), color);
	tempWorldVerts[5] = Vertex_PCU(Vec3(bottomLeft.x, bottomLeft.y, 0), color);

	g_theRenderer->DrawVertexArray(6, tempWorldVerts);
}

void DebugDrawRing(Vec2 const& centerPos, float radius, float thickness, Rgba8 const& color)
{
	float innerRadius = radius - thickness *.5f;
	float outerRadius = radius + thickness * .5f;

	constexpr int NUM_SIDES = 16;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	Vertex_PCU verts[NUM_VERTS];
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float>( NUM_SIDES );
	for (int sideNum = 0; sideNum < NUM_SIDES; ++ sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		//compute inner and outer positions
		Vec3 innerStartPos = Vec3(centerPos.x + innerRadius * CosDegrees(startDegrees), centerPos.y + innerRadius * SinDegrees(startDegrees), 0);
		Vec3 outerStartPos = Vec3(centerPos.x + outerRadius * CosDegrees(startDegrees), centerPos.y + outerRadius * SinDegrees(startDegrees), 0);
		Vec3 outerEndPos = Vec3(centerPos.x + outerRadius * CosDegrees(endDegrees), centerPos.y + outerRadius * SinDegrees(endDegrees), 0);
		Vec3 innerEndPos = Vec3(centerPos.x + innerRadius * CosDegrees(endDegrees), centerPos.y + innerRadius * SinDegrees(endDegrees), 0);

		//Trapezoid is made of two triangels ABC and DEF
		//A is inner end; B is inner start C is outer start
		//D is inner end; E is outer start; F is outer end
		int vertIndexA = 6 * sideNum;
		int vertIndexB = 6 * sideNum + 1;
		int vertIndexC = 6 * sideNum + 2;
		int vertIndexD = 6 * sideNum + 3;
		int vertIndexE = 6 * sideNum + 4;
		int vertIndexF = 6 * sideNum + 5;
		verts[vertIndexA].m_position = innerEndPos;
		verts[vertIndexB].m_position = innerStartPos;
		verts[vertIndexC].m_position = outerStartPos;

		verts[vertIndexA].m_color = color;
		verts[vertIndexB].m_color = color;
		verts[vertIndexC].m_color = color;

		verts[vertIndexD].m_position = innerEndPos;
		verts[vertIndexE].m_position = outerStartPos;
		verts[vertIndexF].m_position = outerEndPos;

		verts[vertIndexD].m_color = color;
		verts[vertIndexE].m_color = color;
		verts[vertIndexF].m_color = color;
	}
	g_theRenderer->DrawVertexArray(NUM_VERTS, verts);

}

void DebugDrawDisc(Vec2 const& centerPos, float radius, Rgba8 const& color)
{
	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;
	Vertex_PCU verts[NUM_VERTS];
	constexpr float DEGREES_PER_SIDE = 360.f / static_cast<float>(NUM_SIDES);
	Vec3 centerVertPos(centerPos.x, centerPos.y, 0.f);
	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);
		
		Vec3 startVertPos = Vec3(CosDegrees(startDegrees), SinDegrees(startDegrees), 0.f) * radius;
		Vec3 endVertPos = Vec3(CosDegrees(endDegrees), SinDegrees(endDegrees), 0.f) * radius;
		startVertPos.x += centerPos.x;
		startVertPos.y += centerPos.y;
		endVertPos.x += centerPos.x;
		endVertPos.y += centerPos.y;

		int startVertIdx = 3 * sideNum;
		int centerVertIdx = 3 * sideNum + 1;
		int endVertIdx = 3 * sideNum + 2;

		verts[startVertIdx].m_position = startVertPos;
		verts[centerVertIdx].m_position = centerVertPos;
		verts[endVertIdx].m_position = endVertPos;

		verts[startVertIdx].m_color = color;
		verts[centerVertIdx].m_color = color;
		verts[endVertIdx].m_color = color;
	}
	g_theRenderer->DrawVertexArray(NUM_VERTS, verts);
}
