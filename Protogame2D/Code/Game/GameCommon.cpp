#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

SoundID SOUND_ID_TEST;

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