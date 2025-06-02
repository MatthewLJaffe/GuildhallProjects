#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/AABB2.hpp"

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int i = 0; i < numVerts; i++)
	{
		TransformPositionXY3D(verts[i].m_position, scaleXY, rotationDegreesAboutZ, translationXY);
	}
}

void TransformVertexArrayXY3D(size_t numVerts, Vertex_PCU* m_localVerts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY)
{
	for (size_t i = 0; i < numVerts; i++)
	{
		TransformPositionXY3D(m_localVerts[i].m_position, iBasis, jBasis, translationXY);
	}
}

void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	Vec2 segmentDisp = boneEnd - boneStart;
	Vec2 iBasis = segmentDisp.GetNormalized();
	Vec2 capsuleCenter = boneStart + .5f * segmentDisp;
	float halfWidth = segmentDisp.GetLength() * .5f;
	float halfHeight = radius;
	AddVertsForOOB2D(verts, OBB2(capsuleCenter, Vec2(halfWidth, halfHeight), iBasis), color);
	AddVertsForOrientedSector2D(verts, boneEnd, iBasis, 180.f, radius, color);
	AddVertsForOrientedSector2D(verts, boneStart, -iBasis, 180.f, radius, color);
}

void AddVertsForOrientedSector2D(std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color)
{
	constexpr int NUM_SIDES = 16;
	float thetaStep = sectorApertureDegrees / static_cast<float>(NUM_SIDES);
	Vec2 prevSideDisp = sectorForwardNormal.GetRotatedDegrees(-.5f * sectorApertureDegrees) * sectorRadius;
	Vec2 currSideDisp;
	for (int i = 0; i < NUM_SIDES; i++)
	{
		currSideDisp = prevSideDisp.GetRotatedDegrees(thetaStep);
		verts.push_back( Vertex_PCU( (currSideDisp + sectorTip).GetXYZ(), color ) );
		verts.push_back( Vertex_PCU( (sectorTip).GetXYZ(), color ) );
		verts.push_back( Vertex_PCU( (prevSideDisp + sectorTip).GetXYZ(), color) );
		prevSideDisp = currSideDisp;
	}
}

void AddVertsForDisc2D(std::vector<Vertex_PCU>& m_localVerts, Vec2 const& centerPos, float radius, Rgba8 const& color, int numSides, float heightOffsetForVaporum)
{
	float DEGREES_PER_SIDE = 360.f / static_cast<float>(numSides);
	Vec3 centerVertPos(centerPos.x, centerPos.y, heightOffsetForVaporum);
	for (int sideNum = 0; sideNum < numSides; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		Vec3 startVertPos = Vec3(CosDegrees(startDegrees), SinDegrees(startDegrees), heightOffsetForVaporum) * radius;
		Vec3 endVertPos = Vec3(CosDegrees(endDegrees), SinDegrees(endDegrees), heightOffsetForVaporum) * radius;
		startVertPos.x += centerPos.x;
		startVertPos.y += centerPos.y;
		endVertPos.x += centerPos.x;
		endVertPos.y += centerPos.y;

		m_localVerts.push_back(Vertex_PCU(centerVertPos, color));
		m_localVerts.push_back(Vertex_PCU(startVertPos, color));
		m_localVerts.push_back(Vertex_PCU(endVertPos, color));
	}
}

void AddVertsForRing2D(std::vector<Vertex_PCU>& m_localVerts, Vec2 const& centerPos, float radius, float thickness, Rgba8 const& color, int numSides, float heightOffsetForVaporum)
{
	float innerRadius = radius - thickness * .5f;
	float outerRadius = radius + thickness * .5f;

	float DEGREES_PER_SIDE = 360.f / static_cast<float>(numSides);
	for (int sideNum = 0; sideNum < numSides; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		//compute inner and outer positions
		Vec3 innerStartPos = Vec3(centerPos.x + innerRadius * CosDegrees(startDegrees), centerPos.y + innerRadius * SinDegrees(startDegrees), heightOffsetForVaporum);
		Vec3 outerStartPos = Vec3(centerPos.x + outerRadius * CosDegrees(startDegrees), centerPos.y + outerRadius * SinDegrees(startDegrees), heightOffsetForVaporum);
		Vec3 outerEndPos = Vec3(centerPos.x + outerRadius * CosDegrees(endDegrees), centerPos.y + outerRadius * SinDegrees(endDegrees), heightOffsetForVaporum);
		Vec3 innerEndPos = Vec3(centerPos.x + innerRadius * CosDegrees(endDegrees), centerPos.y + innerRadius * SinDegrees(endDegrees), heightOffsetForVaporum);

		m_localVerts.push_back(Vertex_PCU(innerEndPos, color));
		m_localVerts.push_back(Vertex_PCU(innerStartPos, color));
		m_localVerts.push_back(Vertex_PCU(outerStartPos, color));

		m_localVerts.push_back(Vertex_PCU(innerEndPos, color));
		m_localVerts.push_back(Vertex_PCU(outerStartPos, color));
		m_localVerts.push_back(Vertex_PCU(outerEndPos, color));
	}
}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, Vec2 const& mins, Vec2 const& maxs, Rgba8 const& color, Vec2 const& uvMins, Vec2 const& uvMaxs)
{
	verts.push_back(Vertex_PCU(Vec3(maxs.x, maxs.y, 0.f), color, Vec2(uvMaxs.x, uvMaxs.y)));
	verts.push_back(Vertex_PCU(Vec3(mins.x, maxs.y, 0.f), color, Vec2(uvMins.x, uvMaxs.y)));
	verts.push_back(Vertex_PCU(Vec3(mins.x, mins.y, 0.f), color, Vec2(uvMins.x, uvMins.y)));

	verts.push_back(Vertex_PCU(Vec3(maxs.x, maxs.y, 0.f), color, Vec2(uvMaxs.x, uvMaxs.y)));
	verts.push_back(Vertex_PCU(Vec3(mins.x, mins.y, 0.f), color, Vec2(uvMins.x, uvMins.y)));
	verts.push_back(Vertex_PCU(Vec3(maxs.x, mins.y, 0.f), color, Vec2(uvMaxs.x, uvMins.y)));
}

void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uvMins, Vec2 const& uvMaxs, float sortOrder)
{
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, sortOrder), color, Vec2(uvMaxs.x, uvMaxs.y)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, sortOrder), color, Vec2(uvMins.x, uvMaxs.y)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, sortOrder), color, Vec2(uvMins.x, uvMins.y)));
																	  
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, sortOrder), color, Vec2(uvMaxs.x, uvMaxs.y)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, sortOrder), color, Vec2(uvMins.x, uvMins.y)));
	verts.push_back(Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, sortOrder), color, Vec2(uvMaxs.x, uvMins.y)));
}

void AddVertsForOOB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color)
{
	Vec2 jBasis = box.m_iBasisNormal.GetRotated90Degrees();
	Vec2 tr = box.m_iBasisNormal * box.m_halfDimensions.x + jBasis * box.m_halfDimensions.y + box.m_center;
	Vec2 tl = -box.m_iBasisNormal * box.m_halfDimensions.x + jBasis * box.m_halfDimensions.y + box.m_center;
	Vec2 br = box.m_iBasisNormal * box.m_halfDimensions.x + -jBasis * box.m_halfDimensions.y + box.m_center;
	Vec2 bl = -box.m_iBasisNormal * box.m_halfDimensions.x + -jBasis * box.m_halfDimensions.y + box.m_center;

	verts.push_back(Vertex_PCU(tr.GetXYZ(), color, Vec2(1, 1)));
	verts.push_back(Vertex_PCU(tl.GetXYZ(), color, Vec2(0, 1)));
	verts.push_back(Vertex_PCU(bl.GetXYZ(), color, Vec2(0, 0)));
	
	verts.push_back(Vertex_PCU(br.GetXYZ(), color, Vec2(1, 0)));
	verts.push_back(Vertex_PCU(tr.GetXYZ(), color, Vec2(1, 1)));
	verts.push_back(Vertex_PCU(bl.GetXYZ(), color, Vec2(0, 0)));
}

void AddVertsForLine2D(std::vector<Vertex_PCU>& verts, Vec2 const& pointA, Vec2 const& pointB, float thickness, Rgba8 const& color)
{
	Vec2 forwardDir = (pointB - pointA).GetNormalized();
	Vec2 leftDir = forwardDir.GetRotated90Degrees();
	Vec2 bottom_Left = pointA - (forwardDir * thickness) + (leftDir * thickness);
	Vec2 bottom_Right = pointA - (forwardDir * thickness) - (leftDir * thickness);
	Vec2 top_Left = pointB + (forwardDir * thickness) + (leftDir * thickness);
	Vec2 top_Right = pointB + (forwardDir * thickness) - (leftDir * thickness);

	verts.push_back(Vertex_PCU(Vec3(bottom_Left.x, bottom_Left.y, 0), color));
	verts.push_back(Vertex_PCU(Vec3(bottom_Right.x, bottom_Right.y, 0), color));
	verts.push_back(Vertex_PCU(Vec3(top_Right.x, top_Right.y, 0), color));

	verts.push_back(Vertex_PCU(Vec3(bottom_Left.x, bottom_Left.y, 0), color));
	verts.push_back(Vertex_PCU(Vec3(top_Right.x, top_Right.y, 0), color));
	verts.push_back(Vertex_PCU(Vec3(top_Left.x, top_Left.y, 0), color));
}


void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	AddVertsForLine2D(verts, tailPos, tipPos, lineThickness, color);
	Vec2 iBasis = (tailPos - tipPos).GetNormalized();
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	AddVertsForLine2D(verts, tipPos + (iBasis * arrowSize) + (jBasis * arrowSize), tipPos, lineThickness, color);
	AddVertsForLine2D(verts, tipPos + (iBasis * arrowSize) - (jBasis * arrowSize), tipPos, lineThickness, color);
}

void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y)));
	verts.push_back(Vertex_PCU(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));

	verts.push_back(Vertex_PCU(bottomLeft, color, UVs.m_mins));
	verts.push_back(Vertex_PCU(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y)));
	verts.push_back(Vertex_PCU(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y)));
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 tangent = (bottomRight - bottomLeft).GetNormalized();
	Vec3 bitangent = (topLeft - bottomLeft).GetNormalized();
	Vec3 normal = CrossProduct3D(tangent, bitangent).GetNormalized();
	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y), tangent, bitangent, normal));

	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y), tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, normal));
}

void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 tangent = bottomRight - bottomLeft;
	Vec3 bitangent = topLeft - bottomLeft;
	Vec3 normal = CrossProduct3D(tangent, bitangent);
	int startIdx = (int)verts.size();
	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topRight, color, Vec2(UVs.m_maxs.x, UVs.m_maxs.y), tangent, bitangent, normal));
	verts.push_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, normal));

	indexes.push_back(0 + startIdx);
	indexes.push_back(1 + startIdx);
	indexes.push_back(2 + startIdx);

	indexes.push_back(0 + startIdx);
	indexes.push_back(2 + startIdx);
	indexes.push_back(3 + startIdx);
}

void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color, const AABB2& UVs)
{
	Vec3 bottomLeftNear = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 bottomRightNear = bounds.m_mins;
	Vec3 topRightNear = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 topLeftNear = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	Vec3 bottomLeftFar = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 bottomRightFar = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 topRightFar = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 topLeftFar = bounds.m_maxs;

	AddVertsForQuad3D(verts, bottomRightFar, bottomLeftFar, topLeftFar, topRightFar, color, UVs);
	AddVertsForQuad3D(verts, bottomLeftNear, bottomRightNear, topRightNear, topLeftNear, color, UVs);

	AddVertsForQuad3D(verts, topLeftNear, topRightNear, topRightFar, topLeftFar, color, UVs);
	AddVertsForQuad3D(verts, bottomRightNear, bottomLeftNear, bottomLeftFar, bottomRightFar, color, UVs);

	AddVertsForQuad3D(verts, bottomLeftFar, bottomLeftNear, topLeftNear, topLeftFar, color, UVs);
	AddVertsForQuad3D(verts, bottomRightNear, bottomRightFar, topRightFar, topRightNear, color, UVs);
}

void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numSlices, int numStacks)
{
	float yawStep = 360.f / numSlices;
	float pitchStep = 180.f / numStacks;

	AABB2 currUVs;

	for (int yawSteps = 0; yawSteps <= numSlices - 1; yawSteps++)
	{
		for (int pitchSteps = 0; pitchSteps <= numStacks - 1; pitchSteps++)
		{
			float yaw = yawStep * (float)yawSteps;
			float pitch = (pitchStep * (float)pitchSteps) - 90.f;

			float leftYaw = yaw;
			float topPitch = pitch;
			float rightYaw = yaw + yawStep;
			float bottomPitch = pitch + pitchStep;

			currUVs.m_mins.x = RangeMap(leftYaw, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			currUVs.m_maxs.x = RangeMap(rightYaw, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			currUVs.m_mins.y = 1.f - RangeMap(bottomPitch, -90, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			currUVs.m_maxs.y = 1.f - RangeMap(topPitch, -90, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			Vec3 bottom_Right = Vec3(CosDegrees(rightYaw)*CosDegrees(bottomPitch) * radius, SinDegrees(rightYaw)*CosDegrees(bottomPitch)*radius, -SinDegrees(bottomPitch)*radius) + center;
			Vec3 top_Right = Vec3(CosDegrees(rightYaw)*CosDegrees(topPitch)*radius, SinDegrees(rightYaw)*CosDegrees(topPitch)*radius, -SinDegrees(topPitch)*radius) + center;
			Vec3 top_Left = Vec3(CosDegrees(leftYaw)*CosDegrees(topPitch)*radius, SinDegrees(leftYaw)*CosDegrees(topPitch)*radius, -SinDegrees(topPitch)*radius) + center;
			Vec3 bottom_Left = Vec3(CosDegrees(leftYaw)*CosDegrees(bottomPitch)*radius, SinDegrees(leftYaw)*CosDegrees(bottomPitch)*radius, -SinDegrees(bottomPitch)*radius) + center;
			AddVertsForQuad3D(verts, bottom_Left, bottom_Right, top_Right, top_Left, color, currUVs);
		}
	}
}

void AddVertsForSphere3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& center, float radius, const Rgba8& color, const AABB2& UVs, int numSlices, int numStacks)
{
	float yawStep = 360.f / numSlices;
	float pitchStep = 180.f / numStacks;

	AABB2 currUVs;

	for (int yawSteps = 0; yawSteps <= numSlices - 1; yawSteps++)
	{
		for (int pitchSteps = 0; pitchSteps <= numStacks - 1; pitchSteps++)
		{
			float yaw = yawStep * (float)yawSteps;
			float pitch = (pitchStep * (float)pitchSteps) - 90.f;

			float leftYaw = yaw;
			float topPitch = pitch;
			float rightYaw = yaw + yawStep;
			float bottomPitch = pitch + pitchStep;

			currUVs.m_mins.x = RangeMap(leftYaw, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			currUVs.m_maxs.x = RangeMap(rightYaw, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			currUVs.m_mins.y = 1.f - RangeMap(bottomPitch, -90, 90.f, UVs.m_mins.y, UVs.m_maxs.y);
			currUVs.m_maxs.y = 1.f - RangeMap(topPitch, -90, 90.f, UVs.m_mins.y, UVs.m_maxs.y);

			Vec3 bottom_Right = Vec3(CosDegrees(rightYaw) * CosDegrees(bottomPitch) * radius, SinDegrees(rightYaw) * CosDegrees(bottomPitch) * radius, -SinDegrees(bottomPitch) * radius) + center;
			Vec3 top_Right = Vec3(CosDegrees(rightYaw) * CosDegrees(topPitch) * radius, SinDegrees(rightYaw) * CosDegrees(topPitch) * radius, -SinDegrees(topPitch) * radius) + center;
			Vec3 top_Left = Vec3(CosDegrees(leftYaw) * CosDegrees(topPitch) * radius, SinDegrees(leftYaw) * CosDegrees(topPitch) * radius, -SinDegrees(topPitch) * radius) + center;
			Vec3 bottom_Left = Vec3(CosDegrees(leftYaw) * CosDegrees(bottomPitch) * radius, SinDegrees(leftYaw) * CosDegrees(bottomPitch) * radius, -SinDegrees(bottomPitch) * radius) + center;
			AddVertsForQuad3D(verts, bottom_Left, bottom_Right, top_Right, top_Left, color, currUVs);
		}
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform)
{
	for (size_t i = 0; i < verts.size(); i++)
	{
		verts[i].m_position = transform.TransformPosition3D(verts[i].m_position);
	}
}

void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform, bool transformNormals)
{
	for (size_t i = 0; i < verts.size(); i++)
	{
		verts[i].m_position = transform.TransformPosition3D(verts[i].m_position);
		if (transformNormals)
		{
			verts[i].m_normal = transform.TransformVectorQuantity3D(verts[i].m_normal);
			verts[i].m_normal = verts[i].m_normal.GetNormalized();
		}
	}
}


AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU> verts)
{
	AABB2 bounds = AABB2(Vec2(9999999.f, 9999999.f), Vec2(-9999999.f, -9999999.f));
	for (size_t i = 0; i < verts.size(); i++)
	{
		if (verts[i].m_position.x < bounds.m_mins.x)
		{
			bounds.m_mins.x = verts[i].m_position.x;
		}
		if (verts[i].m_position.y < bounds.m_mins.y)
		{
			bounds.m_mins.y = verts[i].m_position.y;
		}

		if (verts[i].m_position.x > bounds.m_maxs.x)
		{
			bounds.m_maxs.x = verts[i].m_position.x;
		}
		if (verts[i].m_position.y > bounds.m_maxs.y)
		{
			bounds.m_maxs.y = verts[i].m_position.y;
		}
	}
	
	return bounds;
}

void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8 color, const AABB2& UVs, int numSlices)
{
	std::vector<Vertex_PCU> vertsToTransform;
	UNUSED(UVs);
	float height =  (end - start).GetLength();

	float DEGREES_PER_SIDE = 360.f / static_cast<float>(numSlices);

	//create in local space with bottom at z = 0
	for (int sideNum = 0; sideNum < numSlices; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		//bottom
		Vec3 center = Vec3::ZERO;
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		Vec3 bottomStartVertPos = Vec3(cosStart, sinStart, 0.f) * radius;
		Vec2 startUVs(RangeMap(cosStart, -1.f, 1.f, 0.f, 1.f), RangeMap(sinStart, -1.f, 1.f, 0.f, 1.f));

		float cosEnd = CosDegrees(endDegrees);
		float sinEnd = SinDegrees(endDegrees);
		Vec3 bottomEndVertPos = Vec3(cosEnd, sinEnd, 0.f) * radius;
		Vec2 endUVs(RangeMap(cosEnd, -1.f, 1.f, 0.f, 1.f), RangeMap(sinEnd, -1.f, 1.f, 0.f, 1.f));

		bottomStartVertPos.x += center.x;
		bottomStartVertPos.y += center.y;
		bottomEndVertPos.x += center.x;
		bottomEndVertPos.y += center.y;

		//bottom
		vertsToTransform.push_back(Vertex_PCU(center, color, Vec2(.5f, .5f)));
		vertsToTransform.push_back(Vertex_PCU(bottomEndVertPos, color, endUVs));
		vertsToTransform.push_back(Vertex_PCU(bottomStartVertPos, color, startUVs));

		//Top
		Vec3 topCenter = center + Vec3(0.f, 0.f, height);
		Vec3 topStartVertPos = bottomStartVertPos + Vec3(0.f, 0.f, height);
		Vec3 topEndVertPos = bottomEndVertPos + Vec3(0.f, 0.f, height);

		vertsToTransform.push_back(Vertex_PCU(topCenter, color, Vec2(.5f, .5f)));
		vertsToTransform.push_back(Vertex_PCU(topStartVertPos, color, startUVs));
		vertsToTransform.push_back(Vertex_PCU(topEndVertPos, color, endUVs));

		//wall
		Vec2 wallBottomStartUVs(startDegrees / 360.f, 0.f);
		Vec2 wallBottomEndUVs(endDegrees / 360.f, 0.f);
		Vec2 wallTopStartUVs(startDegrees / 360.f, 1.f);
		Vec2 wallTopEndUVs(endDegrees / 360.f, 1.f);

		vertsToTransform.push_back(Vertex_PCU(bottomStartVertPos, color, wallBottomStartUVs));
		vertsToTransform.push_back(Vertex_PCU(bottomEndVertPos, color, wallBottomEndUVs));
		vertsToTransform.push_back(Vertex_PCU(topEndVertPos, color, wallTopEndUVs));

		vertsToTransform.push_back(Vertex_PCU(bottomStartVertPos, color, wallBottomStartUVs));
		vertsToTransform.push_back(Vertex_PCU(topEndVertPos, color, wallTopEndUVs));
		vertsToTransform.push_back(Vertex_PCU(topStartVertPos, color, wallTopStartUVs));
	}

	//transformation
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 iBasis;
	Vec3 jBasis;
	if (fabsf(fabsf( DotProduct3D(kBasis, Vec3(0.f, 0.f, 1.f)) ) - 1.f) < .01f)
	{
		iBasis = Vec3(1.f, 0.f, 0.f);
		jBasis = Vec3(0.f, 1.f, 0.f);
	}
	else
	{
		iBasis = -CrossProduct3D(Vec3(0.f, 0.f, 1.f), kBasis);
		iBasis = iBasis.GetNormalized();
		jBasis = CrossProduct3D(kBasis, iBasis);
		jBasis = jBasis.GetNormalized();
	}

	Mat44 cylinderSpace;
	cylinderSpace.SetIJKT3D(iBasis, jBasis, kBasis, start);
 	TransformVertexArray3D(vertsToTransform, cylinderSpace);
	for (size_t i = 0; i < vertsToTransform.size(); i++)
	{
		verts.push_back(vertsToTransform[i]);
	}
}

void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, 
	float radius, const Rgba8& color, const AABB2& UVs, int numSlices)
{
	UNUSED(UVs);
	float height = (end - start).GetLength();

	float DEGREES_PER_SIDE = 360.f / static_cast<float>(numSlices);

	//create in local space with bottom at z = 0
	for (int sideNum = 0; sideNum < numSlices; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		//bottom
		Vec3 center = Vec3::ZERO;
		Vec3 bottomStartVertPos = Vec3(CosDegrees(startDegrees), SinDegrees(startDegrees), 0.f) * radius;
		Vec3 bottomEndVertPos = Vec3(CosDegrees(endDegrees), SinDegrees(endDegrees), 0.f) * radius;
		bottomStartVertPos.x += center.x;
		bottomStartVertPos.y += center.y;
		bottomEndVertPos.x += center.x;
		bottomEndVertPos.y += center.y;

		verts.push_back(Vertex_PCU(center, color));
		verts.push_back(Vertex_PCU(bottomEndVertPos, color));
		verts.push_back(Vertex_PCU(bottomStartVertPos, color));

		//tip
		center += Vec3(0.f, 0.f, height);
		verts.push_back(Vertex_PCU(bottomStartVertPos, color));
		verts.push_back(Vertex_PCU(bottomEndVertPos, color));
		verts.push_back(Vertex_PCU(center, color));
	}

	//transformation
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 iBasis;
	Vec3 jBasis;
	if ( fabsf( DotProduct3D(kBasis, Vec3(0.f, 0.f, 1.f)) ) == 1.f )
	{
		iBasis = Vec3(1.f, 0.f, 0.f);
		jBasis = Vec3(0.f, 1.f, 0.f);
	}
	else
	{
		iBasis = -CrossProduct3D(Vec3(0.f, 0.f, 1.f), kBasis);
		iBasis = iBasis.GetNormalized();
		jBasis = CrossProduct3D(kBasis, iBasis);
		jBasis = jBasis.GetNormalized();
	}

	Mat44 coneSpace;
	coneSpace.SetIJKT3D(iBasis, jBasis, kBasis, start);
	TransformVertexArray3D(verts, coneSpace);

}

void AddVertsForEasingFunction(std::vector<Vertex_PCU>& verts, EasingFunctionPtr easingFunctionPtr, Vec2 const& startPos, Vec2 const& endPos, float lineThickness, Rgba8 color, int numSubdivisions)
{
	float currT = 0.f;
	float tStep = 1.f / (float)numSubdivisions;

	for (int i = 0; i < numSubdivisions; i++)
	{
		currT = (float)i / (float)numSubdivisions;
		float nextT = currT + tStep;
		float currEasingT = easingFunctionPtr(currT);
		float nextEasingT = easingFunctionPtr(nextT);

		Vec2 segmentStartPos;
		segmentStartPos.x = Lerp(startPos.x, endPos.x, currT);
		segmentStartPos.y = Lerp(startPos.y, endPos.y, currEasingT);

		Vec2 segmentEndPos;
		segmentEndPos.x = Lerp(startPos.x, endPos.x, nextT);
		segmentEndPos.y = Lerp(startPos.y, endPos.y, nextEasingT);
		AddVertsForLine2D(verts, segmentStartPos, segmentEndPos, lineThickness, color);
	}
}

void AddVertsForCubicBezier(std::vector<Vertex_PCU>& verts, CubicBezierCurve2D const& cubicBezier, float lineThickness, Rgba8 color, int numSubdivisions)
{
	float currT = 0.f;
	float tStep = 1.f / (float)numSubdivisions;

	for (int i = 0; i < numSubdivisions; i++)
	{
		currT = (float)i / (float)numSubdivisions;
		float nextT = currT + tStep;
		Vec2 startSegmentPos = cubicBezier.EvaluateAtParametric(currT);
		Vec2 endSegmentPos = cubicBezier.EvaluateAtParametric(nextT);
		AddVertsForLine2D(verts, startSegmentPos, endSegmentPos, lineThickness, color);
	}
}

void AddVertsForCubicHermite(std::vector<Vertex_PCU>& verts, CubicHermiteCurve2D const& cubicHermite, float lineThickness, Rgba8 color, int numSubdivisions)
{
	float currT = 0.f;
	float tStep = 1.f / (float)numSubdivisions;

	for (int i = 0; i < numSubdivisions; i++)
	{
		currT = (float)i / (float)numSubdivisions;
		float nextT = currT + tStep;
		Vec2 startSegmentPos = cubicHermite.EvaluateAtParametric(currT);
		Vec2 endSegmentPos = cubicHermite.EvaluateAtParametric(nextT);
		AddVertsForLine2D(verts, startSegmentPos, endSegmentPos, lineThickness, color);
	}
}

void AddVertsForCatmullRomSpline(std::vector<Vertex_PCU>& verts, CatmullRomSpline const& catmulRomSpline, float lineThickness, Rgba8 color, int numSubdivisions)
{
	std::vector<Vec2> positions = catmulRomSpline.GetPositions();
	std::vector<Vec2> velocities = catmulRomSpline.GetVelocities();
	for (int i = 0; i < (int)positions.size() - 1; i++)
	{
		CubicHermiteCurve2D hermite(positions[i], velocities[i], positions[i + 1], velocities[i + 1]);
		AddVertsForCubicHermite(verts, hermite, lineThickness, color, numSubdivisions);
	}
}

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& color, const AABB2& UVs)
{
	Vec3 rightNormal = bottomRight - bottomLeft;
	Vec3 leftNormal = -rightNormal;
	Vec3 bottomMid = (bottomLeft + bottomRight) * .5f;
	Vec3 topMid = (topLeft + topRight) * .5f;

	Vec3 tangent = bottomRight - bottomLeft;
	Vec3 bitangent = topLeft - bottomLeft;
	Vec3 normal = CrossProduct3D(tangent, bitangent);
	float uvMidX = (UVs.m_maxs.x + UVs.m_mins.x) * .5f;

	//left quad
	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, Vec3::ZERO, Vec3::ZERO, leftNormal));
	verts.push_back(Vertex_PCUTBN(bottomMid, color, Vec2(uvMidX, UVs.m_mins.y), Vec3::ZERO, Vec3::ZERO, normal));
	verts.push_back(Vertex_PCUTBN(topMid, color, Vec2(uvMidX, UVs.m_maxs.y), Vec3::ZERO, Vec3::ZERO, normal));

	verts.push_back(Vertex_PCUTBN(bottomLeft, color, UVs.m_mins, tangent, bitangent, leftNormal));
	verts.push_back(Vertex_PCUTBN(topMid, color, Vec2(uvMidX, UVs.m_maxs.y), Vec3::ZERO, Vec3::ZERO, normal));
	verts.push_back(Vertex_PCUTBN(topLeft, color, Vec2(UVs.m_mins.x, UVs.m_maxs.y), tangent, bitangent, leftNormal));

	//right quad
	verts.push_back(Vertex_PCUTBN(bottomMid, color, Vec2(uvMidX, UVs.m_mins.y), Vec3::ZERO, Vec3::ZERO, normal));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, Vec2(UVs.m_maxs.x, UVs.m_mins.y), Vec3::ZERO, Vec3::ZERO, rightNormal));
	verts.push_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, Vec3::ZERO, Vec3::ZERO, rightNormal));

	verts.push_back(Vertex_PCUTBN(bottomMid, color, Vec2(uvMidX, UVs.m_mins.y), Vec3::ZERO, Vec3::ZERO, normal));
	verts.push_back(Vertex_PCUTBN(topRight, color, UVs.m_maxs, Vec3::ZERO, Vec3::ZERO, rightNormal));
	verts.push_back(Vertex_PCUTBN(topMid, color, Vec2(uvMidX, UVs.m_maxs.y), Vec3::ZERO, Vec3::ZERO, normal));
}

void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, Rgba8 color)
{
	Vec3 endToStartDir = (start - end).GetNormalized();
	Vec3 cylinderEndPos = end + (endToStartDir * 3.f * radius);
	AddVertsForCylinder3D(verts, start, cylinderEndPos, radius, color, AABB2::ZERO_TO_ONE, 32);
	std::vector<Vertex_PCU> coneVerts;
	coneVerts.reserve(128);
	AddVertsForCone3D(coneVerts, cylinderEndPos, end, radius * 1.75f, color, AABB2::ZERO_TO_ONE, 32);
	for (size_t i = 0; i < coneVerts.size(); i++)
	{
		verts.push_back(coneVerts[i]);
	}
}

void AddVertsForLine3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8 startColor, const Rgba8 endColor, const AABB2& UVs, int numSlices)
{
	std::vector<Vertex_PCU> vertsToTransform;
	UNUSED(UVs);
	float height = (end - start).GetLength();

	float DEGREES_PER_SIDE = 360.f / static_cast<float>(numSlices);

	//create in local space with bottom at z = 0
	for (int sideNum = 0; sideNum < numSlices; ++sideNum)
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum);
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>(sideNum + 1);

		//bottom
		Vec3 center = Vec3::ZERO;
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		Vec3 bottomStartVertPos = Vec3(cosStart, sinStart, 0.f) * radius;
		Vec2 startUVs(RangeMap(cosStart, -1.f, 1.f, 0.f, 1.f), RangeMap(sinStart, -1.f, 1.f, 0.f, 1.f));

		float cosEnd = CosDegrees(endDegrees);
		float sinEnd = SinDegrees(endDegrees);
		Vec3 bottomEndVertPos = Vec3(cosEnd, sinEnd, 0.f) * radius;
		Vec2 endUVs(RangeMap(cosEnd, -1.f, 1.f, 0.f, 1.f), RangeMap(sinEnd, -1.f, 1.f, 0.f, 1.f));

		bottomStartVertPos.x += center.x;
		bottomStartVertPos.y += center.y;
		bottomEndVertPos.x += center.x;
		bottomEndVertPos.y += center.y;

		//bottom
		vertsToTransform.push_back(Vertex_PCU(center, startColor, Vec2(.5f, .5f)));
		vertsToTransform.push_back(Vertex_PCU(bottomEndVertPos, startColor, endUVs));
		vertsToTransform.push_back(Vertex_PCU(bottomStartVertPos, startColor, startUVs));

		//Top
		Vec3 topCenter = center + Vec3(0.f, 0.f, height);
		Vec3 topStartVertPos = bottomStartVertPos + Vec3(0.f, 0.f, height);
		Vec3 topEndVertPos = bottomEndVertPos + Vec3(0.f, 0.f, height);

		vertsToTransform.push_back(Vertex_PCU(topCenter, endColor, Vec2(.5f, .5f)));
		vertsToTransform.push_back(Vertex_PCU(topStartVertPos, endColor, startUVs));
		vertsToTransform.push_back(Vertex_PCU(topEndVertPos, endColor, endUVs));

		//wall
		Vec2 wallBottomStartUVs(startDegrees / 360.f, 0.f);
		Vec2 wallBottomEndUVs(endDegrees / 360.f, 0.f);
		Vec2 wallTopStartUVs(startDegrees / 360.f, 1.f);
		Vec2 wallTopEndUVs(endDegrees / 360.f, 1.f);

		vertsToTransform.push_back(Vertex_PCU(bottomStartVertPos, startColor, wallBottomStartUVs));
		vertsToTransform.push_back(Vertex_PCU(bottomEndVertPos, startColor, wallBottomEndUVs));
		vertsToTransform.push_back(Vertex_PCU(topEndVertPos, endColor, wallTopEndUVs));

		vertsToTransform.push_back(Vertex_PCU(bottomStartVertPos, startColor, wallBottomStartUVs));
		vertsToTransform.push_back(Vertex_PCU(topEndVertPos, endColor, wallTopEndUVs));
		vertsToTransform.push_back(Vertex_PCU(topStartVertPos, endColor, wallTopStartUVs));
	}

	//transformation
	Vec3 kBasis = (end - start).GetNormalized();
	Vec3 iBasis;
	Vec3 jBasis;
	if (fabsf(DotProduct3D(kBasis, Vec3(0.f, 0.f, 1.f))) == 1.f)
	{
		iBasis = Vec3(1.f, 0.f, 0.f);
		jBasis = Vec3(0.f, 1.f, 0.f);
	}
	else
	{
		iBasis = -CrossProduct3D(Vec3(0.f, 0.f, 1.f), kBasis);
		iBasis = iBasis.GetNormalized();
		jBasis = CrossProduct3D(kBasis, iBasis);
		jBasis = jBasis.GetNormalized();
	}

	Mat44 cylinderSpace;
	cylinderSpace.SetIJKT3D(iBasis, jBasis, kBasis, start);
	TransformVertexArray3D(vertsToTransform, cylinderSpace);
	for (size_t i = 0; i < vertsToTransform.size(); i++)
	{
		verts.push_back(vertsToTransform[i]);
	}
}

void AddVertsForBoxLine3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, const Rgba8 color)
{
	Vec3 center = (start + end) * .5f;
	Vec3 iBasis = (end - start).GetNormalized();
	float halfDimX = (end - start).GetLength() * .5f;
	Vec3 jBasis;
	Vec3 kBasis;
	CreateBasisVectorsFromIBasis(iBasis, jBasis, kBasis);

	OBB3 boxLine(iBasis, jBasis, kBasis, Vec3(halfDimX, radius, radius), center);
	AddVertsForOBB3(verts, boxLine, color);
}

void AddVertsForBasis3D(std::vector<Vertex_PCU>& verts, Mat44 const& basis)
{
	float uniformScale = basis.GetIBasis3D().GetLength();
	AddVertsForArrow3D(verts, basis.GetTranslation3D(), basis.GetTranslation3D() + basis.GetIBasis3D(), .05f * uniformScale, Rgba8::RED);
	AddVertsForArrow3D(verts, basis.GetTranslation3D(), basis.GetTranslation3D() + basis.GetJBasis3D(), .05f * uniformScale, Rgba8::GREEN);
	AddVertsForArrow3D(verts, basis.GetTranslation3D(), basis.GetTranslation3D() + basis.GetKBasis3D(), .05f * uniformScale, Rgba8::BLUE);
	AddVertsForSphere3D(verts, basis.GetTranslation3D(), uniformScale * .1f);
}

void AddVertsForOBB3(std::vector<Vertex_PCU>& verts, OBB3 obb3, Rgba8 const& color)
{
	std::vector<Vertex_PCU> localVerts;
	AABB3 localBox(-obb3.m_halfDimensions, obb3.m_halfDimensions);
	AddVertsForAABB3D(localVerts, localBox, color);
	Mat44 localToWorld;
	localToWorld.SetIJKT3D(obb3.m_iBasis, obb3.m_jBasis, obb3.m_kBasis, obb3.m_center);

	TransformVertexArray3D(localVerts, localToWorld);
	for (size_t i = 0; i < localVerts.size(); i++)
	{
		verts.push_back(localVerts[i]);
	}
}

void AddVertsForBoxEdges(std::vector<Vertex_PCU>& verts, AABB3 const& box, float lineWidth, Rgba8 const& color)
{
	UNUSED(color);
	Vec3 boxDimensions = box.GetDimensions();
	Vec3 boxCenter = box.GetCenter();
	//emission box
	float boxHalfLength = boxDimensions.x * .5f;
	float boxHalfWidth = boxDimensions.y * .5f;
	float boxHalfHeight = boxDimensions.z * .5f;

	Vec3 boxBackLeftBottom = Vec3(-boxHalfLength, boxHalfWidth, -boxHalfHeight) + boxCenter;
	Vec3 boxBackRightBottom = Vec3(-boxHalfLength, -boxHalfWidth, -boxHalfHeight) + boxCenter;
	Vec3 boxBackRightTop = Vec3(-boxHalfLength, -boxHalfWidth, boxHalfHeight) + boxCenter;
	Vec3 boxBackLeftTop = Vec3(-boxHalfLength, boxHalfWidth, boxHalfHeight) + boxCenter;
	Vec3 boxForwardLeftBottom = Vec3(boxHalfLength, boxHalfWidth, -boxHalfHeight) + boxCenter;
	Vec3 boxForwardRightBottom = Vec3(boxHalfLength, -boxHalfWidth, -boxHalfHeight) + boxCenter;
	Vec3 boxForwardRightTop = Vec3(boxHalfLength, -boxHalfWidth, boxHalfHeight) + boxCenter;
	Vec3 boxForwardLeftTop = Vec3(boxHalfLength, boxHalfWidth, boxHalfHeight) + boxCenter;

	AddVertsForBoxLine3D(verts, boxBackLeftBottom, boxBackRightBottom, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxBackRightBottom, boxBackRightTop, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxBackRightTop, boxBackLeftTop, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxBackLeftTop, boxBackLeftBottom, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxBackLeftBottom, boxForwardLeftBottom, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxBackRightBottom, boxForwardRightBottom, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxForwardLeftBottom, boxForwardRightBottom, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxForwardRightBottom, boxForwardRightTop, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxForwardRightTop, boxForwardLeftTop, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxForwardRightTop, boxBackRightTop, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxForwardLeftTop, boxForwardLeftBottom, lineWidth, Rgba8::WHITE);
	AddVertsForBoxLine3D(verts, boxForwardLeftTop, boxBackLeftTop, lineWidth, Rgba8::WHITE);
}

void AddVertsForPyramid3D(std::vector<Vertex_PCUTBN>& verts, Vec3 const& basePosition, float width, float height, Rgba8 const& color)
{
	Vec3 backLeft = basePosition + Vec3(-width, width, 0.f);
	Vec3 backRight = basePosition + Vec3(-width, -width, 0.f);
	Vec3 frontRight = basePosition + Vec3(width, -width, 0.f);
	Vec3 frontLeft = basePosition + Vec3(width, width, 0.f);
	Vec3 top = basePosition + Vec3(0.f, 0.f, height);

	Vec3 frontNormal = CrossProduct3D((frontRight - frontLeft), (top - frontLeft)).GetNormalized();
	Vec3 rightNormal = CrossProduct3D((frontRight - backRight), (top - backRight)).GetNormalized();
	Vec3 leftNormal = CrossProduct3D(backLeft - frontLeft, top - frontLeft).GetNormalized();
	Vec3 backNormal = CrossProduct3D((backLeft - backRight), (top - backRight)).GetNormalized();


	AddVertsForQuad3D(verts, backRight, backLeft, frontLeft, frontRight);

	
	verts.push_back(Vertex_PCUTBN(backLeft, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, frontNormal));
	verts.push_back(Vertex_PCUTBN(backRight, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, frontNormal));
	verts.push_back(Vertex_PCUTBN(top, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, frontNormal));

	verts.push_back(Vertex_PCUTBN(backRight, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, rightNormal));
	verts.push_back(Vertex_PCUTBN(frontRight, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, rightNormal));
	verts.push_back(Vertex_PCUTBN(top, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, rightNormal));

	verts.push_back(Vertex_PCUTBN(frontLeft, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, leftNormal));
	verts.push_back(Vertex_PCUTBN(backLeft, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, leftNormal));
	verts.push_back(Vertex_PCUTBN(top, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, leftNormal));

	verts.push_back(Vertex_PCUTBN(frontRight, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, backNormal));
	verts.push_back(Vertex_PCUTBN(frontLeft, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, backNormal));
	verts.push_back(Vertex_PCUTBN(top, color, Vec2::ZERO, Vec3::ZERO, Vec3::ZERO, backNormal));
}

void AddVertsForCubeMapSkyBox(std::vector<Vertex_PCU>& verts, Vec3 const& centerPositoin, float sideLength, Rgba8 const& color)
{
	AABB3 cubeBounds(Vec3(centerPositoin) - .5*Vec3(sideLength, sideLength, sideLength), Vec3(centerPositoin) + .5 * Vec3(sideLength, sideLength, sideLength));
	Vec3 bottomLeftNear = Vec3(cubeBounds.m_mins.x, cubeBounds.m_maxs.y, cubeBounds.m_mins.z);
	Vec3 bottomRightNear = cubeBounds.m_mins;
	Vec3 topRightNear = Vec3(cubeBounds.m_mins.x, cubeBounds.m_mins.y, cubeBounds.m_maxs.z);
	Vec3 topLeftNear = Vec3(cubeBounds.m_mins.x, cubeBounds.m_maxs.y, cubeBounds.m_maxs.z);

	Vec3 bottomLeftFar = Vec3(cubeBounds.m_maxs.x, cubeBounds.m_maxs.y, cubeBounds.m_mins.z);
	Vec3 bottomRightFar = Vec3(cubeBounds.m_maxs.x, cubeBounds.m_mins.y, cubeBounds.m_mins.z);
	Vec3 topRightFar = Vec3(cubeBounds.m_maxs.x, cubeBounds.m_mins.y, cubeBounds.m_maxs.z);
	Vec3 topLeftFar = cubeBounds.m_maxs;

	AABB2 frontUVs(.25f, .333f, .5f, .666f);
	AABB2 leftUVs(0.f ,.334f, .25f, .665f);
	AABB2 rightUVs(.5f, .334f, .75f, .665f);
	AABB2 topUVs(.251f, .666f, .499f, 1.f);
	AABB2 bottomUVs(.251f, .001f, .499f, .333f);
	AABB2 backUVs(.75f, .334f, 1.f, .666f);

	//front quad
	AddVertsForQuad3D(verts, bottomLeftFar, bottomRightFar, topRightFar, topLeftFar, color, frontUVs);

	//back quad
	AddVertsForQuad3D(verts, bottomRightNear, bottomLeftNear, topLeftNear, topRightNear, color, backUVs);

	//top quad
	AddVertsForQuad3D(verts, topLeftFar, topRightFar, topRightNear, topLeftNear, color, topUVs);

	//bottom quad
	AddVertsForQuad3D(verts, bottomLeftNear, bottomRightNear, bottomRightFar, bottomLeftFar, color, bottomUVs);

	//left quad
	AddVertsForQuad3D(verts, bottomLeftNear, bottomLeftFar, topLeftFar, topLeftNear, color, leftUVs);

	//right quad
	AddVertsForQuad3D(verts, bottomRightFar, bottomRightNear, topRightNear, topRightFar, color, rightUVs);
}
