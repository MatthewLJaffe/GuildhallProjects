#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Plane2.hpp"

float ConvertDegreesToRadians(float degrees)
{
	return degrees * PI / 180.f;
}

float ConvertRadiansToDegrees(float radians)
{
	return radians * 180.f / PI;
}

float CosDegrees(float degrees)
{
	return cosf(ConvertDegreesToRadians(degrees));
}

float SinDegrees(float degrees)
{
	return sinf(ConvertDegreesToRadians(degrees));
}

float TanDegrees(float degrees)
{
	return tanf(ConvertDegreesToRadians(degrees));
}
float Atan2Degrees(float y, float x)
{
	return ConvertRadiansToDegrees(atan2f(y, x));
}

float GetShortestAngularDispDegrees(float currentDegrees, float goalDegrees)
{
	float disp = goalDegrees - currentDegrees;
	while (disp > 180.f)
	{
		disp -= 360.f;
	}
	while (disp < -180.f)
	{
		disp += 360.f;
	}
	return disp;
}

float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	float shortestDisp = GetShortestAngularDispDegrees(currentDegrees, goalDegrees);
	if (fabsf(shortestDisp) < maxDeltaDegrees)
		return goalDegrees;
	if (shortestDisp > 0.f)
		return currentDegrees + maxDeltaDegrees;
	return currentDegrees - maxDeltaDegrees;
}

float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	float aDotb = DotProduct2D(a, b);
	float cosTheta = aDotb / (a.GetLength() * b.GetLength());
	return ConvertRadiansToDegrees(acosf(cosTheta));
}

float GetAngleDegreesBetweenVectors3D(Vec3 const& a, Vec3 const& b)
{
	float aDotb = DotProduct3D(a, b);
	float cosTheta = aDotb / (a.GetLength() * b.GetLength());
	return ConvertRadiansToDegrees(acosf(cosTheta));
}

float GetUnsignedAngleBetweenVectors3D(Vec3 const& a, Vec3 const& b)
{
	float ABMag = a.GetLength() * b.GetLength();
	float dot = DotProduct3D(a, b);
	float signedAngleRadians = acosf(dot / ABMag);
	return fabsf(ConvertRadiansToDegrees(signedAngleRadians));
}

// Basic 2D and 3D utilities
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	return (positionA - positionB).GetLength();
}

float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	float xDiff = positionA.x - positionB.x;
	float yDiff = positionA.y - positionB.y;
	return xDiff * xDiff + yDiff * yDiff;
}

float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	return (positionA - positionB).GetLength();
}

float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float xDiff = positionA.x - positionB.x;
	float yDiff = positionA.y - positionB.y;
	float zDiff = positionA.z - positionB.z;
	return xDiff * xDiff + yDiff * yDiff + zDiff * zDiff;
}

float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float xDiff = positionA.x - positionB.x;
	float yDiff = positionA.y - positionB.y;
	return sqrtf(xDiff * xDiff + yDiff * yDiff);
}

float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	float xDiff = positionA.x - positionB.x;
	float yDiff = positionA.y - positionB.y;
	return xDiff * xDiff + yDiff * yDiff;
}

int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	return abs(pointA.x - pointB.x) + abs(pointA.y - pointB.y);
}

float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	return DotProduct2D(vectorToProject, vectorToProjectOnto.GetNormalized());
}

Vec2 const GetProjectedOnto2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 projectOntoNormal = vectorToProjectOnto.GetNormalized();
	float length = DotProduct2D(vectorToProject, projectOntoNormal);
	return projectOntoNormal * length;
}

Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	Vec3 projectOntoNormal = vectorToProjectOnto.GetNormalized();
	float length = DotProduct3D(vectorToProject, projectOntoNormal);
	return projectOntoNormal * length;
}

Vec2 const GetNearestPointOnDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	if (IsPointInsideDisc2D(point, discCenter, discRadius))
	{
		return point;
	}
	Vec2 discToPointDisp = point - discCenter;
	return discCenter + discToPointDisp.GetNewLength(discRadius);
}

Vec2 const GetNearestPointOnRing2D(Vec2 const& point, Vec2 const& ringCenter, float ringRadius)
{
	Vec2 discToPointDisp = point - ringCenter;
	return ringCenter + discToPointDisp.GetNewLength(ringRadius);
}

Vec2 const GetNearestPointOnAABB2D(Vec2 const& point, AABB2 const& box)
{
	return box.GetNearestPoint(point);
}

Vec2 const GetNearestPointOnOBB2D(Vec2 const& point, OBB2 const& box)
{
	Vec2 boxToPointDisp = point - box.m_center;
	Vec2 jBasis = box.m_iBasisNormal.GetRotated90Degrees();
	float iPos = DotProduct2D(box.m_iBasisNormal, boxToPointDisp);
	float jPos = DotProduct2D(jBasis, boxToPointDisp);
	iPos = Clamp(iPos, -box.m_halfDimensions.x, box.m_halfDimensions.x);
	jPos = Clamp(jPos, -box.m_halfDimensions.y, box.m_halfDimensions.y);
	return box.m_center + iPos * box.m_iBasisNormal + jPos * jBasis;
}

Vec2 const GetNearestPointOnCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 segmentPoint = GetNearestPointOnLineSegment2D(point, boneStart, boneEnd);
	return segmentPoint + (point - segmentPoint).Clamp(radius);
}

Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& point, Vec2 const& lineStart, Vec2 const& lineEnd)
{
	Vec2 dispStartToPoint = point - lineStart;
	Vec2 endToStartDir = lineStart - lineEnd;
	if (DotProduct2D(dispStartToPoint, endToStartDir) > 0)
	{
		return lineStart;
	}

	Vec2 dispEndToPoint = point - lineEnd;
	Vec2 startToEndDir = -endToStartDir;
	if (DotProduct2D(dispEndToPoint, startToEndDir) > 0)
	{
		return lineEnd;
	}

	Vec2 iBasis = startToEndDir.GetNormalized();
	float iPos = DotProduct2D(iBasis, dispStartToPoint);
	return lineStart + iPos * iBasis;
}

Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& point, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 iBasis = (anotherPointOnLine - pointOnLine).GetNormalized();
	Vec2 dispFromLineToPoint = point - pointOnLine;
	return pointOnLine + iBasis * DotProduct2D(iBasis, dispFromLineToPoint);
}

float const GetDistanceFromLineSegment2D(Vec2 const& point, Vec2 const& lineStart, Vec2 const& lineEnd)
{
	Vec2 dispStartToPoint = point - lineStart;
	Vec2 endToStartDir = lineStart - lineEnd;
	if (DotProduct2D(dispStartToPoint, endToStartDir) > 0)
	{
		return dispStartToPoint.GetLength();
	}

	Vec2 dispEndToPoint = point - lineEnd;
	Vec2 startToEndDir = -endToStartDir;
	if (DotProduct2D(dispEndToPoint, startToEndDir) > 0)
	{
		return dispEndToPoint.GetLength();
	}

	Vec2 jBasis = startToEndDir.GetNormalized().GetRotated90Degrees();
	float jPos = DotProduct2D(jBasis, dispStartToPoint);
	return fabsf(jPos);
}

float const GetDistanceFromLineSegment3D(Vec3 const& point, Vec3 const& lineStart, Vec3 const& lineEnd)
{
	Vec3 startToEndDisp = lineEnd - lineStart;
	Vec3 startToPointDisp = point - lineStart;
	Vec3 startToEndDir = startToEndDisp.GetNormalized();
	float distanceAlongLine = DotProduct3D(startToEndDir, startToPointDisp);
	if (distanceAlongLine < 0.f)
	{
		return GetDistance3D(point, lineStart);
	}
	if (distanceAlongLine > DotProduct3D(startToEndDir, startToEndDisp))
	{
		return GetDistance3D(point, lineEnd);
	}
	Vec3 awayFromLineDisp = startToPointDisp - distanceAlongLine * startToEndDir;
	return awayFromLineDisp.GetLength();
}

float const GetDistanceSquaredFromLineSegment3D(Vec3 const& point, Vec3 const& lineStart, Vec3 const& lineEnd)
{
	Vec3 startToEndDisp = lineEnd - lineStart;
	Vec3 startToPointDisp = point - lineStart;
	Vec3 startToEndDir = startToEndDisp.GetNormalized();
	float distanceAlongLine = DotProduct3D(startToEndDir, startToPointDisp);
	if (distanceAlongLine < 0.f)
	{
		return GetDistanceSquared3D(point, lineStart);
	}
	if (distanceAlongLine > DotProduct3D(startToEndDir, startToEndDisp))
	{
		return GetDistanceSquared3D(point, lineEnd);
	}
	Vec3 awayFromLineDisp = startToPointDisp - distanceAlongLine * startToEndDir;
	return awayFromLineDisp.GetLengthSquared();
}

Vec3 const GetNearestPointOnSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius)
{
	if (IsPointInsideSphere3D(point, sphereCenter, sphereRadius))
	{
		return point;
	}
	Vec3 newDispFromCenter = point - sphereCenter;
	newDispFromCenter.SetLength(sphereRadius);
	return sphereCenter + newDispFromCenter;
}

Vec3 const GetNearestPointOnCylinder3D(Vec3 const& point, Vec2 const& xyCenter, float radius, FloatRange zMinMax)
{
	Vec2 nearestXY = GetNearestPointOnDisc2D(Vec2(point.x, point.y), xyCenter, radius);
	float nearestZ = Clamp(point.z, zMinMax.m_min, zMinMax.m_max);
	return Vec3(nearestXY.x, nearestXY.y, nearestZ);
}

Vec3 const GetNearestPointOnAABB3D(Vec3 const& point, AABB3 const& box)
{
	Vec3 clampedPoint = Vec3(
		Clamp(point.x, box.m_mins.x, box.m_maxs.x),
		Clamp(point.y, box.m_mins.y, box.m_maxs.y),
		Clamp(point.z, box.m_mins.z, box.m_maxs.z));
	return clampedPoint;
}

Vec3 const GetNearestPointOnOBB3(Vec3 const& point, OBB3 const& box)
{
	AABB3 localBox(-box.m_halfDimensions, box.m_halfDimensions);
	Mat44 localToWorldMatrix;
	localToWorldMatrix.SetIJKT3D(box.m_iBasis, box.m_jBasis, box.m_kBasis, box.m_center);
	Mat44 worldToLocalMatrix = localToWorldMatrix.GetOrthonormalInverse();

	Vec3 pointInLocalSpace = worldToLocalMatrix.TransformPosition3D(point);
	Vec3 nearestPointLocal = GetNearestPointOnAABB3D(pointInLocalSpace, localBox);
	Vec3 nearestPoint = localToWorldMatrix.TransformPosition3D(nearestPointLocal);
	return nearestPoint;
}

Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& targetMatrix, const Vec3& billboardPosition, const Vec2& billboardScale)
{
	Mat44 transformation;
	Vec3 targetPosition = targetMatrix.GetTranslation3D();
	transformation.AppendTranslation3D(billboardPosition);
	transformation.AppendScaleNonUniform2D(billboardScale);

	if (billboardType == BillboardType::FULL_OPPOSING)
	{
		transformation.SetIJK3D(-targetMatrix.GetIBasis3D(), -targetMatrix.GetJBasis3D(), targetMatrix.GetKBasis3D());
	}

	else if (billboardType == BillboardType::FULL_FACING)
	{
		Vec3 fullFacingIBasis = (targetPosition - billboardPosition).GetNormalized();
		Vec3 fullFacingJBasis;
		Vec3 fullFacingKBasis;

		if (fabsf(DotProduct3D(fullFacingIBasis, Vec3(0.f, 0.f, 1.f))) != 1.f)
		{
			fullFacingJBasis = CrossProduct3D(Vec3(0.f, 0.f, 1.f), fullFacingIBasis).GetNormalized() * billboardScale.x;
			fullFacingKBasis = CrossProduct3D(fullFacingIBasis, fullFacingJBasis).GetNormalized() * billboardScale.y;
			transformation.SetIJK3D(fullFacingIBasis, fullFacingJBasis, fullFacingKBasis);
		}
		else
		{
			fullFacingJBasis = CrossProduct3D(Vec3(0.f, 1.f, 0.f), fullFacingIBasis) * billboardScale.x;
			fullFacingKBasis = CrossProduct3D(fullFacingIBasis, fullFacingJBasis) * billboardScale.y;
		}

		transformation.SetIJK3D(fullFacingIBasis, fullFacingJBasis, fullFacingKBasis);
		return transformation;
	}

	else if (billboardType == BillboardType::WORLD_UP_FACING)
	{
		Vec3 worldUpFacingIBasis;
		Vec3 worldUpFacingJBasis;
		Vec3 worldUpFacingKBasis;

		worldUpFacingKBasis = Vec3(0.f, 0.f, 1.f);

		worldUpFacingIBasis = targetPosition - billboardPosition;
		worldUpFacingIBasis.z = 0.f;
		worldUpFacingIBasis = worldUpFacingIBasis.GetNormalized();

		worldUpFacingJBasis = Vec3(-worldUpFacingIBasis.y, worldUpFacingIBasis.x, 0.f);
		transformation.SetIJK3D(worldUpFacingIBasis, worldUpFacingJBasis, worldUpFacingKBasis);
		return transformation;
	}

	else if (billboardType == BillboardType::WORLD_UP_OPPOSING)
	{
		Vec3 worldUpOpposingIBasis;
		Vec3 worldUpOpposingJBasis;
		Vec3 worldUpOpposingKBasis;

		worldUpOpposingKBasis = Vec3(0.f, 0.f, 1.f);

		worldUpOpposingIBasis = -targetMatrix.GetIBasis3D();
		worldUpOpposingIBasis.z = 0.f;
		worldUpOpposingIBasis = worldUpOpposingIBasis.GetNormalized();

		worldUpOpposingJBasis = CrossProduct3D(worldUpOpposingKBasis, worldUpOpposingIBasis);
		transformation.SetIJK3D(worldUpOpposingIBasis, worldUpOpposingJBasis, worldUpOpposingKBasis);
		return transformation;
	}
	else if (billboardType == BillboardType::NONE)
	{
		return transformation;
	}

	return transformation;
}

Mat44 GetRotationMatrixFromIBasisKUp(Vec3 const& iBasis)
{
	Mat44 rotationMatrix;
	Vec3 fullFacingJBasis;
	Vec3 fullFacingKBasis;

	if (fabsf(DotProduct3D(iBasis, Vec3(0.f, 0.f, 1.f))) != 1.f)
	{
		fullFacingJBasis = CrossProduct3D(Vec3(0.f, 0.f, 1.f), iBasis).GetNormalized();
		fullFacingKBasis = CrossProduct3D(iBasis, fullFacingJBasis).GetNormalized();
		rotationMatrix.SetIJK3D(iBasis, fullFacingJBasis, fullFacingKBasis);
	}
	else
	{
		fullFacingJBasis = CrossProduct3D(Vec3(0.f, 1.f, 0.f), iBasis).GetNormalized();
		fullFacingKBasis = CrossProduct3D(iBasis, fullFacingJBasis).GetNormalized();
	}

	rotationMatrix.SetIJK3D(iBasis, fullFacingJBasis, fullFacingKBasis);

	return rotationMatrix;
}

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	return GetDistanceSquared2D(point, discCenter) < discRadius * discRadius;
}

bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	return box.IsPointInside(point);
}

bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) > sectorRadius)
	{
		return false;
	}

	float tipToPointDegrees = (point - sectorTip).GetOrientationDegrees();
	float angleDifference = fabsf(GetShortestAngularDispDegrees(tipToPointDegrees, sectorForwardDegrees));
	return angleDifference < sectorApertureDegrees * .5f;
}

bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	if (GetDistance2D(point, sectorTip) > sectorRadius)
	{
		return false;
	}

	float tipToPointDegrees = (point - sectorTip).GetOrientationDegrees();
	float sectorForwardDegrees = sectorForwardNormal.GetOrientationDegrees();
	float angleDifference = fabsf(GetShortestAngularDispDegrees(tipToPointDegrees, sectorForwardDegrees));
	return angleDifference < sectorApertureDegrees * .5f;
}

bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox)
{
	Vec2 dispToPoint = point - orientedBox.m_center;
	float iPos = DotProduct2D(orientedBox.m_iBasisNormal, dispToPoint);
	if (iPos > orientedBox.m_halfDimensions.x)
	{
		return false;
	}
	if (iPos < -orientedBox.m_halfDimensions.x)
	{
		return false;
	}
	Vec2 jBasisNormal = orientedBox.m_iBasisNormal.GetRotated90Degrees();
	float jPos = DotProduct2D(jBasisNormal, dispToPoint);
	if (jPos > orientedBox.m_halfDimensions.y)
	{
		return false;
	}
	if (jPos < -orientedBox.m_halfDimensions.y)
	{
		return false;
	}
	return true;
}


bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	float distanceToSeg = GetDistanceFromLineSegment2D(point, boneStart, boneEnd);
	return distanceToSeg <= radius;
}

bool IsPointInsideConvexHull2D(Vec2 const& point, ConvexHull2D const& convexHull)
{
	for (int i = 0; i < (int)convexHull.m_boundingPlanes.size(); i++)
	{
		if (convexHull.m_boundingPlanes[i].GetAltitude(point) >= 0.f)
		{
			return false;
		}
	}
	return true;
}

//Geometric query utilities

bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	return GetDistanceSquared2D(centerA, centerB) < (radiusA + radiusB) * (radiusA + radiusB);
}

bool DoAABBsOverlap2D(AABB2 const& box1, AABB2 const& box2)
{
	if (box1.m_maxs.x <= box2.m_mins.x || box1.m_mins.x >= box2.m_maxs.x ||
		box1.m_maxs.y <= box2.m_mins.y || box1.m_mins.y >= box2.m_maxs.y)
	{
		return false;
	}
	return true;
}

bool DoDiscAndAABBOverlap2D(Vec2 const& discCenter, float discRadius, AABB2 const& aabb2)
{
	return IsPointInsideDisc2D(aabb2.GetNearestPoint(discCenter), discCenter, discRadius);
}

bool DoDiscAndOBBOverlap2D(Vec2 const& discCenter, float discRadius, OBB2 const& obb2)
{
	return IsPointInsideDisc2D(GetNearestPointOnOBB2D(discCenter, obb2), discCenter, discRadius);
}

bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	return GetDistance3D(centerA, centerB) <= radiusA + radiusB;
}

bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedPoint)
{
	if (GetDistance2D(mobileDiscCenter, fixedPoint) > mobileDiscRadius)
	{
		return false;
	}
	Vec2 dispDiscToPoint = fixedPoint - mobileDiscCenter;
	Vec2 edgeToDiscDisp = -dispDiscToPoint.GetNormalized() * mobileDiscRadius;
	Vec2 discDisplacment = dispDiscToPoint + edgeToDiscDisp;
	mobileDiscCenter += discDisplacment;
	return true;
}

bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	float apartDistance = mobileDiscRadius + fixedDiscRadius;
	float apartDistanceSquared = apartDistance * apartDistance;
	if (GetDistanceSquared2D(mobileDiscCenter, fixedDiscCenter) > apartDistanceSquared)
	{
		return false;
	}

	Vec2 dispFixedToMobile = mobileDiscCenter - fixedDiscCenter;
	float distanceToDisplace = apartDistance - dispFixedToMobile.GetLength();
	mobileDiscCenter += dispFixedToMobile.GetNormalized() * distanceToDisplace;
	return true;
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	Vec2 atoBDisp = bCenter - aCenter;
	Vec2 targetAToBDisp = atoBDisp.GetNewLength(aRadius + bRadius);
	if (atoBDisp.GetLength() >= aRadius + bRadius)
	{
		return false;
	}

	Vec2 dispToPushA = .5f * (atoBDisp - targetAToBDisp);
	aCenter += dispToPushA;
	Vec2 dispToPushB = -dispToPushA;
	bCenter += dispToPushB;
	return true;
}

bool PushDiscOutOfFixedAABB2D(Vec2& discCenter, float discRadius, AABB2 const& fixedBox)
{
	return PushDiscOutOfFixedPoint2D(discCenter, discRadius, fixedBox.GetNearestPoint(discCenter));
}

bool BounceMobileDiscOffFixedPoint2D(Vec2& discCenter, Vec2& discVelocity, float discRadius, Vec2 const& fixedPoint, float elasticity)
{
	//too far to bounce
	if (GetDistanceSquared2D(discCenter, fixedPoint) > discRadius * discRadius)
	{
		return false;
	}
	PushDiscOutOfFixedPoint2D(discCenter, discRadius, fixedPoint);
	Vec2 impactNormal = (discCenter - fixedPoint).GetNormalized();
	Vec2 discMoveDir = discVelocity.GetNormalized();
	//going away already should not bounce
	if (DotProduct2D(impactNormal, discMoveDir) > 0.f)
	{
		return false;
	}

	discVelocity.Reflect(impactNormal, elasticity);
	return true;
}

bool BounceMobileDiscsOffEachOther2D(Vec2& pos1, Vec2& vel1, float radius1, Vec2& pos2, Vec2& vel2, float radius2, float elasticity)
{
	float radiusSum = radius1 + radius2;
	if (GetDistanceSquared2D(pos1, pos2) > radiusSum * radiusSum)
	{
		return false;
	}

	PushDiscsOutOfEachOther2D(pos1, radius1, pos2, radius2);

	Vec2 iBasis = (pos2 - pos1).GetNormalized();
	Vec2 jBasis = iBasis.GetRotated90Degrees();

	float v1I = DotProduct2D(vel1, iBasis);
	float v1J = DotProduct2D(vel1, jBasis);

	float v2I = DotProduct2D(vel2, iBasis);
	float v2J = DotProduct2D(vel2, jBasis);

	if (v2I - v1I > 0.f)
	{
		return false;
	}

	vel1 = (v2I * iBasis * elasticity) + v1J * jBasis;
	vel2 = (v1I * iBasis * elasticity) + v2J * jBasis;
	return true;
}

bool DoAABBsOverlap3D(AABB3 const& first, AABB3 const& second)
{
	if (first.m_mins.x > second.m_maxs.x || first.m_maxs.x < second.m_mins.x ||
		first.m_mins.y > second.m_maxs.y || first.m_maxs.y < second.m_mins.y ||
		first.m_mins.z > second.m_maxs.z || first.m_maxs.z < second.m_mins.z)
	{
		return false;
	}
	return true;
}

bool DoSpheresOverlap3D(Vec3 const& firstPos, float firstRadius, Vec3 const& secondPos, float secondRadius)
{
	return GetDistanceSquared3D(firstPos, secondPos) < (firstRadius + secondRadius) * (firstRadius + secondRadius);
}

bool DoZCylindersOverlap(Vec2 const& firstXYCenter, float firstRadius, FloatRange firstZMinMax, Vec2 const& secondXYCenter, float secondRadius, FloatRange secondZMinMax)
{
	if (firstZMinMax.m_min > secondZMinMax.m_max || firstZMinMax.m_max < secondZMinMax.m_min)
	{
		return false;
	}
	return DoDiscsOverlap(firstXYCenter, firstRadius, secondXYCenter, secondRadius);
}

bool DoSphereAndAABBOverlap3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box)
{
	Vec3 nearestPointOnAABB3 = GetNearestPointOnAABB3D(sphereCenter, box);
	if (GetDistanceSquared3D(nearestPointOnAABB3, sphereCenter) < sphereRadius * sphereRadius)
	{
		return true;
	}
	return false;
}

bool DoZCylinderAndAABBOverlap3D(Vec2 const& cylinderXYCenter, float cylinderRadius, FloatRange cylinderZMinMax, AABB3 const& box)
{
	//XZ plane
	AABB2 xzCylinder(Vec2(cylinderXYCenter.x - cylinderRadius, cylinderZMinMax.m_min), Vec2(cylinderXYCenter.x + cylinderRadius, cylinderZMinMax.m_max));
	AABB2 xzAABB(Vec2(box.m_mins.x, box.m_mins.z), Vec2(box.m_maxs.x, box.m_maxs.z));
	if (!DoAABBsOverlap2D(xzCylinder, xzAABB))
	{
		return false;
	}

	//YZ plane
	AABB2 yzCylinder(Vec2(cylinderXYCenter.y - cylinderRadius, cylinderZMinMax.m_min), Vec2(cylinderXYCenter.y + cylinderRadius, cylinderZMinMax.m_max));
	AABB2 yzAABB(Vec2(box.m_mins.y, box.m_mins.z), Vec2(box.m_maxs.y, box.m_maxs.z));
	if (!DoAABBsOverlap2D(xzCylinder, xzAABB))
	{
		return false;
	}

	//XY plane
	AABB2 xyAABB(Vec2(box.m_mins.x, box.m_mins.y), Vec2(box.m_maxs.x, box.m_maxs.y));
	if (!DoDiscAndAABBOverlap2D(cylinderXYCenter, cylinderRadius, xyAABB))
	{
		return false;
	}
	return true;
}

//#ToDo Do Z cylinder and Sphere Overlap
bool DoZCylinderAndSphereOverlap3D(Vec2 const& cylinderXYCenter, float cylinderRadius, FloatRange cylinderZMinMax, Vec3 const& sphereCenter, float sphereRadius)
{
	Vec3 nearestPointToSphereOnCylinder = GetNearestPointOnCylinder3D(sphereCenter, cylinderXYCenter, cylinderRadius, cylinderZMinMax);
	if (GetDistanceSquared3D(nearestPointToSphereOnCylinder, sphereCenter) < sphereRadius * sphereRadius)
	{
		return true;
	}
	return false;
}

bool DoPlaneAndOBB3Overlap3D(Plane3 const& plane, OBB3 const& obb3)
{
	std::vector<Vec3> obbCorners = obb3.GetCorners();
	bool firstCornerInFront = plane.IsPointInFrontOfPlane(obbCorners[0]);
	for (size_t i = 1; i < obbCorners.size(); i++)
	{
		//OBB3 must straddle the plane
		if (plane.IsPointInFrontOfPlane(obbCorners[i]) != firstCornerInFront)
		{
			return true;
		}
	}
	return false;
}

bool IsPointInsideSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius)
{
	return GetDistanceSquared3D(point, sphereCenter) < sphereRadius * sphereRadius;
}

bool IsPointInsizeZCylinder3D(Vec3 const& point, Vec2 const& cylinderXYCenter, float cylinderRadius, FloatRange cylinderZMinMax)
{
	if (!cylinderZMinMax.IsOnRange(point.z))
	{
		return false;
	}
	if (!IsPointInsideDisc2D(Vec2(point.x, point.y), cylinderXYCenter, cylinderRadius))
	{
		return false;
	}
	return true;
}

bool IsPointInsideOBB3(Vec3 const& point, OBB3 const& obb3)
{
	Vec3 dispToPoint = point - obb3.m_center;
	float iPos = DotProduct3D(obb3.m_iBasis, dispToPoint);
	if (iPos > obb3.m_halfDimensions.x)
	{
		return false;
	}
	if (iPos < -obb3.m_halfDimensions.x)
	{
		return false;
	}

	float jPos = DotProduct3D(obb3.m_jBasis, dispToPoint);
	if (jPos > obb3.m_halfDimensions.y)
	{
		return false;
	}
	if (jPos < -obb3.m_halfDimensions.y)
	{
		return false;
	}

	float kPos = DotProduct3D(obb3.m_kBasis, dispToPoint);
	if (kPos > obb3.m_halfDimensions.z)
	{
		return false;
	}
	if (kPos < -obb3.m_halfDimensions.z)
	{
		return false;
	}
	return true;
}

bool IsPointInsideCapsule3D(Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius)
{
	float distanceToSeg = GetDistanceSquaredFromLineSegment3D(point, boneStart, boneEnd);
	return distanceToSeg <= radius * radius;
}

void CreateBasisVectorsFromIBasis(Vec3 const& iBasis, Vec3& outJbasis, Vec3& outKBasis)
{
	Vec3 zUp = Vec3(0.f, 0.f, 1.f);
	if (fabsf(DotProduct3D(iBasis, zUp)) != 1.f)
	{
		outJbasis = CrossProduct3D(iBasis, zUp);
		outJbasis = outJbasis.GetNormalized();
	}
	else
	{
		outJbasis = CrossProduct3D(iBasis, Vec3(0.f, 1.f, 0.f));
		outJbasis = outJbasis.GetNormalized();
	}
	outKBasis = CrossProduct3D(iBasis, outJbasis);
	outKBasis = outKBasis.GetNormalized();
}

bool PushSpheresOutOfEachOther3D(Vec3& aCenter, float aRadius, Vec3& bCenter, float bRadius)
{
	Vec3 atoBDisp = bCenter - aCenter;
	Vec3 targetAToBDisp = atoBDisp.GetNewLength(aRadius + bRadius);
	if (atoBDisp.GetLength() >= aRadius + bRadius)
	{
		return false;
	}

	Vec3 dispToPushA = .5f * (atoBDisp - targetAToBDisp);
	aCenter += dispToPushA;
	Vec3 dispToPushB = -dispToPushA;
	bCenter += dispToPushB;
	return true;
}

bool PushSphereOutOfFixedSphere3D(Vec3 const& fixedSphereCenter, float fixedSphereRadius, Vec3& mobileSphereCenter, float mobileSphereRadius)
{
	float apartDistance = mobileSphereRadius + fixedSphereRadius;
	float apartDistanceSquared = apartDistance * apartDistance;
	if (GetDistanceSquared3D(fixedSphereCenter, mobileSphereCenter) > apartDistanceSquared)
	{
		return false;
	}

	Vec3 dispFixedToMobile = mobileSphereCenter - fixedSphereCenter;
	float distanceToDisplace = apartDistance - dispFixedToMobile.GetLength();
	mobileSphereCenter += dispFixedToMobile.GetNormalized() * distanceToDisplace;
	return true;
}

//Transform utilities
void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation)
{
	Vec2& pos = posToTransform;
	float r = sqrtf(pos.x * pos.x + pos.y * pos.y);
	r *= uniformScale;
	float theta = atan2f(pos.y, pos.x);
	theta += rotationDegrees * 3.14159f / 180.f;
	pos.x = r * cosf(theta);
	pos.y = r * sinf(theta);
	pos.y += translation.y;
	pos.x += translation.x;
}

void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	posToTransform = translation + (iBasis * posToTransform.x) + (jBasis * posToTransform.y);
}

void TransformPositionXY3D(Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	Vec3& pos = positionToTransform;
	float r = sqrtf(pos.x * pos.x + pos.y * pos.y);
	r *= scaleXY;
	float theta = atan2f(pos.y, pos.x);
	theta += zRotationDegrees * 3.14159f / 180.f;
	pos.x = r * cosf(theta);
	pos.y = r * sinf(theta);
	pos.x += translationXY.x;
	pos.y += translationXY.y;
}

void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	float oldZ = posToTransform.z;
	posToTransform = Vec3(translation.x, translation.y, 0.f) + Vec3(iBasis.x, iBasis.y, 0.f) * posToTransform.x + Vec3(jBasis.x, jBasis.y, 0.f) * posToTransform.y;
	posToTransform.z = oldZ;
}

float Lerp(float a, float b, float t)
{
	return a * (1.f - t) + b * t;
}

float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	float range = rangeEnd - rangeStart;
	return (value - rangeStart) / range;
}

float GetFractionWithinRange(float value, FloatRange floatRange)
{
	float range = floatRange.m_max - floatRange.m_min;
	return (value - floatRange.m_min) / range;
}

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float t = GetFractionWithinRange(inValue, inStart, inEnd);
	return Lerp(outStart, outEnd, t);
}

float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float t = GetFractionWithinRange(inValue, inStart, inEnd);
	return Lerp(outStart, outEnd, ClampZeroToOne(t));
}

int RoundDownToInt(float value)
{
	if (value < 0)
		value--;
	return static_cast<int>(value);
}

float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	return (a.x * b.x) + (a.y * b.y);
}

float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	float angleBetweenAB = b.GetOrientationDegrees() - a.GetOrientationDegrees();
	float sinOfAngle = SinDegrees(angleBetweenAB);
	return a.GetLength() * b.GetLength() * sinOfAngle;
}

Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	return Vec3(a.y * b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

RaycastResult2D RaycastVsDisc2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius)
{
	RaycastResult2D result;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = maxDist;

	Vec2 iBasis = fwdNormal;
	Vec2 jBasis = iBasis.GetRotated90Degrees();
	Vec2 dispStartToDisc = discCenter - startPos;
	float dispToCenterJPos = DotProduct2D(jBasis, dispStartToDisc);
	if (dispToCenterJPos > discRadius || dispToCenterJPos < -discRadius)
	{
		return result;
	}

	float dispToCenterIPos = DotProduct2D(iBasis, dispStartToDisc);
	float extendedImpactDist = sqrtf(discRadius*discRadius - dispToCenterJPos*dispToCenterJPos);
	Vec2 impactExtendedToCenter = (iBasis * dispToCenterIPos) + startPos;

	//ray was not long enough
	float impactDist = dispToCenterIPos - extendedImpactDist;
	if (impactDist > maxDist)
	{
		return result;
	}

	//ray start is inside disc
	if (IsPointInsideDisc2D(startPos, discCenter, discRadius))
	{
		result.m_impactNormal = -fwdNormal;
		result.m_didImpact = true;
		result.m_impactPos = startPos;
		return result;
	}

	//disc is behind ray 
	if (impactDist < 0)
	{
		return result;
	}

	//normal hit
	result.m_impactDist = dispToCenterIPos - extendedImpactDist;
	result.m_impactPos =  impactExtendedToCenter - (iBasis *  extendedImpactDist);
	result.m_impactNormal = (result.m_impactPos - discCenter).GetNormalized();
	result.m_didImpact = true;
	return result;
}

RaycastResult2D RaycastVsLineSegment2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 lineStart, Vec2 lineEnd)
{
	RaycastResult2D result;

	Vec2 dispToStart = lineStart - startPos;
	Vec2 dispToEnd = lineEnd - startPos;
	
	Vec2 rayIBasis = fwdNormal;
	Vec2 rayJBasis = fwdNormal.GetRotated90Degrees();
	float startJPos = DotProduct2D(dispToStart, rayJBasis);
	float endJPos = DotProduct2D(dispToEnd, rayJBasis);

	//line segment does not straddle ray miss
	if (startJPos * endJPos >= 0.f)
	{
		return result;
	}

	float t = startJPos / (startJPos - endJPos);
	Vec2 impactPoint = Vec2::Lerp(lineStart, lineEnd, t);
	Vec2 dispToImpact = impactPoint - startPos;
	float impactDistance = DotProduct2D(dispToImpact, rayIBasis);
	result.m_impactDist = impactDistance;
	result.m_impactPos = impactPoint;
	//miss from too near
	if (impactDistance < 0)
	{
		return result;
	}
	//miss from too far
	if (impactDistance > maxDist)
	{
		return result;
	}
	result.m_didImpact = true;
	Vec2 lineDir = (lineEnd - lineStart).GetNormalized();
	Vec2 impactNormal = lineDir.GetRotated90Degrees();
	if (endJPos < 0)
	{
		impactNormal *= -1.f;
	}
	result.m_impactNormal = impactNormal;
	return result;
}

RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 const& aabb2)
{
	RaycastResult2D result;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayMaxLength = maxDist;

	Vec2 endPos = startPos + (fwdNormal * maxDist);
	float oneOverRangeX = 1.f / (endPos.x - startPos.x);

	float tMinX = (aabb2.m_mins.x - startPos.x) * oneOverRangeX;
	float tMaxX = (aabb2.m_maxs.x - startPos.x) * oneOverRangeX;
	FloatRange tXRange;
	if (tMinX < tMaxX)
	{
		tXRange = FloatRange(tMinX, tMaxX);
	}
	else
	{
		tXRange = FloatRange(tMaxX, tMinX);
	}
	
	float oneOverRangeY = 1.f / (endPos.y - startPos.y);

	float tMinY = (aabb2.m_mins.y - startPos.y) * oneOverRangeY;
	float tMaxY = (aabb2.m_maxs.y - startPos.y) * oneOverRangeY;
	FloatRange tYRange;
	if (tMinY < tMaxY)
	{
		tYRange = FloatRange(tMinY, tMaxY);
	}
	else
	{
		tYRange = FloatRange(tMaxY, tMinY);
	}
	if (aabb2.IsPointInside(startPos))
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = startPos;
		result.m_impactNormal = -fwdNormal;
		return result;
	}

	//miss
	if (!tXRange.IsOverlappingWith(tYRange) && !tYRange.IsOverlappingWith(tXRange))
	{
		return result;
	}
	//Use greater of the two mins for the overlap T
	float overlapT = tXRange.m_min;
	if (overlapT < tYRange.m_min)
	{
		overlapT = tYRange.m_min;
	}

	//miss
	if (overlapT < 0 || overlapT > 1)
	{
		return result;
	}

	//compute normal
	if (overlapT == tMinX)
	{
		result.m_impactNormal = Vec2::LEFT;
	}
	if (overlapT == tMaxX)
	{
		result.m_impactNormal = Vec2::RIGHT;
	}
	if (overlapT == tMinY)
	{
		result.m_impactNormal = Vec2::DOWN;
	}
	if (overlapT == tMaxY)
	{
		result.m_impactNormal = Vec2::UP;
	}

	result.m_didImpact = true;
	result.m_impactDist = Lerp(0, maxDist, overlapT);
	result.m_impactPos = Vec2::Lerp(startPos, endPos, overlapT);
	return result;
}

RaycastResult2D RaycastVSConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2D const& convexHull)
{
	RaycastResult2D convexHullResult;
	float lastEntryDistance = -1.f;
	float firstExitDistance = maxDist;
	Vec2 lastEntryFwdNormal;
	for (int i = 0; i < (int)convexHull.m_boundingPlanes.size(); i++)
	{
		Plane2 const& currentPlane = convexHull.m_boundingPlanes[i];
		RaycastResult2D currRaycastResult = RaycastVsPlane2D(startPos, fwdNormal, maxDist, currentPlane);
		if (currRaycastResult.m_didImpact)
		{
			float altitude = currentPlane.GetAltitude(startPos);
			//best entry check
			if (altitude < 0.f && currRaycastResult.m_impactDist < firstExitDistance)
			{
				firstExitDistance = currRaycastResult.m_impactDist;
			}
			else if (altitude > 0.f && currRaycastResult.m_impactDist > lastEntryDistance)
			{
				lastEntryDistance = currRaycastResult.m_impactDist;
				lastEntryFwdNormal = currRaycastResult.m_impactNormal;
			}
		}
	}

	//miss by not entering and exiting in right order
	if (lastEntryDistance > firstExitDistance)
	{
		return convexHullResult;
	}
	if (lastEntryDistance < 0)
	{
		//miss by never straddling any boundary and not being inside
		if (!IsPointInsideConvexHull2D(startPos, convexHull))
		{
			return convexHullResult;
		}

		//inside hit
		convexHullResult.m_didImpact = true;
		convexHullResult.m_impactPos = startPos;
		convexHullResult.m_impactNormal = -fwdNormal;
		return convexHullResult;
	}

	Vec2 midPoint = startPos + (fwdNormal * (lastEntryDistance + firstExitDistance) * .5f);
	if (IsPointInsideConvexHull2D(midPoint, convexHull))
	{
		convexHullResult.m_didImpact = true;
		convexHullResult.m_impactDist = lastEntryDistance;
		convexHullResult.m_impactPos = lastEntryDistance * fwdNormal + startPos;
		convexHullResult.m_impactNormal = lastEntryFwdNormal;
		return convexHullResult;
	}

	return convexHullResult;
}

RaycastResult2D RaycastVsPlane2D(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayLength, Plane2 const& plane)
{
	RaycastResult2D result;
	result.m_rayFwdNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;
	result.m_rayStartPos = rayStart;

	float startAltitude = plane.GetAltitude(rayStart);
	float endAltitude = plane.GetAltitude(rayStart + rayForwardNormal * rayLength);

	//did not cross plane
	if (startAltitude * endAltitude > 0)
	{
		return result;
	}

	result.m_didImpact = true;
	float crossT = fabsf(startAltitude) / fabsf(startAltitude - endAltitude);
	result.m_impactPos = rayStart + rayForwardNormal * rayLength * crossT;
	result.m_impactDist = rayLength * crossT;
	if (startAltitude > 0)
	{
		result.m_impactNormal = plane.m_normal;
	}
	else
	{
		result.m_impactNormal = -plane.m_normal;
	}
	return result;
}


RaycastResult3D RaycastVsAABB3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayLength, AABB3 const& box)
{	
	RaycastResult3D result;
	result.m_rayFwdNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;

	Vec3 rayEnd = rayStart + (rayForwardNormal * rayLength);
	//#ToDo handle divide by zero
	float oneOverRangeX = 1.f / (rayEnd.x - rayStart.x);

	float tMinX = (box.m_mins.x - rayStart.x) * oneOverRangeX;
	float tMaxX = (box.m_maxs.x - rayStart.x) * oneOverRangeX;
	FloatRange tXRange;
	if (tMinX < tMaxX)
	{
		tXRange = FloatRange(tMinX, tMaxX);
	}
	else
	{
		tXRange = FloatRange(tMaxX, tMinX);
	}

	float oneOverRangeY = 1.f / (rayEnd.y - rayStart.y);

	float tMinY = (box.m_mins.y - rayStart.y) * oneOverRangeY;
	float tMaxY = (box.m_maxs.y - rayStart.y) * oneOverRangeY;
	FloatRange tYRange;
	if (tMinY < tMaxY)
	{
		tYRange = FloatRange(tMinY, tMaxY);
	}
	else
	{
		tYRange = FloatRange(tMaxY, tMinY);
	}

	float oneOverRangeZ = 1.f / (rayEnd.z - rayStart.z);

	float tMinZ = (box.m_mins.z - rayStart.z) * oneOverRangeZ;
	float tMaxZ = (box.m_maxs.z - rayStart.z) * oneOverRangeZ;
	FloatRange tZRange;
	if (tMinZ < tMaxZ)
	{
		tZRange = FloatRange(tMinZ, tMaxZ);
	}
	else
	{
		tZRange = FloatRange(tMaxZ, tMinZ);
	}

	if (box.IsPointInside(rayStart))
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactPos = rayStart;
		result.m_impactNormal = -rayForwardNormal;
		return result;
	}

	//miss
	if (!tXRange.IsOverlappingWith(tYRange) || !tYRange.IsOverlappingWith(tZRange) || !tXRange.IsOverlappingWith(tZRange))
	{
		return result;
	}
	//Use greater of the mins for the overlap T
	float overlapT = tXRange.m_min;
	if (overlapT < tYRange.m_min)
	{
		overlapT = tYRange.m_min;
	}
	if (overlapT < tZRange.m_min)
	{
		overlapT = tZRange.m_min;
	}

	//miss
	if (overlapT < 0 || overlapT > 1)
	{
		return result;
	}

	//compute normal
	if (overlapT == tMinX)
	{
		result.m_impactNormal = Vec3(-1.f, 0.f, 0.f);
	}
	if (overlapT == tMaxX)
	{
		result.m_impactNormal = Vec3(1.f, 0.f, 0.f);
	}
	if (overlapT == tMinY)
	{
		result.m_impactNormal = Vec3(0.f, -1.f, 0.f);
	}
	if (overlapT == tMaxY)
	{
		result.m_impactNormal = Vec3(0.f, 1.f, 0.f);
	}
	if (overlapT == tMinZ)
	{
		result.m_impactNormal = Vec3(0.f, 0.f, -1.f);
	}
	if (overlapT == tMaxZ)
	{
		result.m_impactNormal = Vec3(0.f, 0.f, 1.f);
	}

	result.m_didImpact = true;
	result.m_impactDist = Lerp(0, rayLength, overlapT);
	result.m_impactPos = Vec3::Lerp(rayStart, rayEnd, overlapT);
	return result;
}

RaycastResult3D RaycastVsSphere3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayLength, Vec3 const& spherePos, float radius)
{
	RaycastResult3D result;
	result.m_rayFwdNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;

	Vec3 dispToCenter = spherePos - rayStart;
	float projectedCenterLength = DotProduct3D(dispToCenter, rayForwardNormal);

	//too short
	if (projectedCenterLength - radius > rayLength)
	{
		return result;
	}

	//inside
	if (IsPointInsideSphere3D(rayStart, spherePos, radius))
	{
		result.m_didImpact = true;
		result.m_impactDist = 0.f;
		result.m_impactNormal = -rayForwardNormal;
		result.m_impactPos = rayStart;
		return result;
	}

	//wrong way
	if (projectedCenterLength < 0)
	{
		return result;
	}

	float dispToCenterLength = dispToCenter.GetLength();
	float perpToCenterLengthSquared = (dispToCenterLength * dispToCenterLength) - (projectedCenterLength * projectedCenterLength);

	//miss
	if (perpToCenterLengthSquared > radius * radius)
	{
		return result;
	}
	float perpToCenterLength = sqrtf(perpToCenterLengthSquared);

	float backUpDistance = sqrtf((radius * radius) - (perpToCenterLength * perpToCenterLength));
	float impactLength = projectedCenterLength - backUpDistance;
	
	//definitely too short this time
	if (impactLength > rayLength)
	{
		return result;
	}

	result.m_didImpact = true;
	result.m_impactDist = impactLength;
	result.m_impactPos = rayStart + impactLength * rayForwardNormal;
	result.m_impactNormal = (result.m_impactPos -  spherePos).GetNormalized();
	return result;
}

RaycastResult3D RaycastVsCylinderZ3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayLength, Vec2 const& centerXY, float radiusXY, FloatRange zMinmax)
{
	RaycastResult3D raycastResult;
	raycastResult.m_rayStartPos = rayStart;
	raycastResult.m_rayMaxLength = rayLength;
	raycastResult.m_rayFwdNormal = rayForwardNormal;

	//get overlap with z range
	Vec3 rayEndPoint = rayStart + rayForwardNormal * rayLength;
	float oneOverRangeZ = 1.f / (rayEndPoint.z - rayStart.z);
	float zMinT = (zMinmax.m_min - rayStart.z) * oneOverRangeZ;
	float zMaxT = (zMinmax.m_max - rayStart.z) * oneOverRangeZ;

	FloatRange zOverlapRange;
	if (zMinT < zMaxT)
	{
		zOverlapRange = FloatRange(zMinT, zMaxT);
	}
	else
	{
		zOverlapRange = FloatRange(zMaxT, zMinT);
	}

	//miss
	if (!zOverlapRange.IsOverlappingWith(FloatRange(0.f, 1.f)))
	{
		return raycastResult;
	}

	//start inside case
	if (IsPointInsizeZCylinder3D(rayStart, centerXY, radiusXY, zMinmax))
	{
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = 0.f;
		raycastResult.m_impactNormal = -rayForwardNormal;
		raycastResult.m_impactPos = rayStart;
		return raycastResult;
	}
	
	//get overlap with sphere range
	Vec2 iBasis = Vec2(rayForwardNormal.x, rayForwardNormal.y);
	Vec2 jBasis = iBasis.GetRotated90Degrees();

	Vec2 dispFromRayStartToDiscCenter = centerXY - Vec2(rayStart.x, rayStart.y);
	Vec2 ijDisplacment(GetProjectedLength2D(dispFromRayStartToDiscCenter, iBasis), GetProjectedLength2D(dispFromRayStartToDiscCenter, jBasis));
	float distFromPerpToImpactPoint = sqrtf(radiusXY * radiusXY - ijDisplacment.y * ijDisplacment.y);
	float discEnterLength = ijDisplacment.x - distFromPerpToImpactPoint;
	float discExitLength = ijDisplacment.x + distFromPerpToImpactPoint;

	Vec2 rayXY = Vec2(rayForwardNormal.x * rayLength, rayForwardNormal.y * rayLength);
	float rayXYLength = rayXY.GetLength();
	float oneOverRayXYLength = 1.f / rayXYLength;

	FloatRange discOverlapRange(discEnterLength * oneOverRayXYLength, discExitLength * oneOverRayXYLength);
	if (!discOverlapRange.IsOverlappingWith(zOverlapRange))
	{
		return raycastResult;
	}

	float impactT = zOverlapRange.m_min;
	if (impactT < discOverlapRange.m_min)
	{
		impactT = discOverlapRange.m_min;
	}
	if (impactT < 0.f || impactT > 1.f)
	{
		return raycastResult;
	}

	//set normal
	if (impactT == zMinT)
	{
		raycastResult.m_impactNormal = Vec3(0.f, 0.f, -1.f);
	}
	else if (impactT == zMaxT)
	{
		raycastResult.m_impactNormal = Vec3(0.f, 0.f, 1.f);
	}
	else
	{
		Vec2 impactPosXY = Vec2::Lerp(Vec2(rayStart.x, rayStart.y), Vec2(rayEndPoint.x, rayEndPoint.y), discOverlapRange.m_min);
		Vec2 impactNormalXY =  impactPosXY - centerXY;
		raycastResult.m_impactNormal = Vec3(impactNormalXY.x, impactNormalXY.y, 0.f).GetNormalized();
	}

	raycastResult.m_didImpact = true;
	raycastResult.m_impactPos = Vec3::Lerp(rayStart, rayEndPoint, impactT);
	raycastResult.m_impactDist = impactT * rayLength;
	return raycastResult;
}

RaycastResult3D RaycastVsPlanes3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayLength, Plane3 const& plane)
{
	RaycastResult3D result;
	result.m_rayFwdNormal = rayForwardNormal;
	result.m_rayMaxLength = rayLength;
	result.m_rayStartPos = rayStart;

	float startAltitude = plane.GetAltitude(rayStart);
	float endAltitude = plane.GetAltitude(rayStart + rayForwardNormal*rayLength);

	//did not cross plane
	if (startAltitude * endAltitude > 0)
	{
		return result;
	}

	result.m_didImpact = true;
	float crossT = fabsf(startAltitude) / fabsf(startAltitude - endAltitude);
	result.m_impactPos = rayStart + rayForwardNormal * rayLength * crossT;
	result.m_impactDist = rayLength * crossT;
	if (startAltitude > 0)
	{
		result.m_impactNormal = plane.m_normal;
	}
	else
	{
		result.m_impactNormal = -plane.m_normal;
	}
	return result;
}

RaycastResult3D RaycastVsOBB3(Vec3 const& rayStart, Vec3 const& rayForwardnormal, float rayLength, OBB3 const& obb3)
{
	AABB3 localBox(-obb3.m_halfDimensions, obb3.m_halfDimensions);
	Mat44 localToWorld;
	localToWorld.SetIJKT3D(obb3.m_iBasis, obb3.m_jBasis, obb3.m_kBasis, obb3.m_center);
	Mat44 worldToLocal = localToWorld.GetOrthonormalInverse();

	Vec3 localRayStart = worldToLocal.TransformPosition3D(rayStart);
	Vec3 localRayNormal = worldToLocal.TransformVectorQuantity3D(rayForwardnormal);

	RaycastResult3D result = RaycastVsAABB3D(localRayStart, localRayNormal, rayLength, localBox);
	result.m_rayStartPos = rayStart;
	result.m_rayFwdNormal = rayForwardnormal;
	result.m_impactNormal = localToWorld.TransformVectorQuantity3D(result.m_impactNormal);
	result.m_impactPos = localToWorld.TransformPosition3D(result.m_impactPos);
	return result;
}

float ComputeCubicBezier1D(float a, float b, float c, float d, float t)
{
	float AB = Lerp(a, b, t);
	float BC = Lerp(b, c, t);
	float CD = Lerp(c, d, t);

	float ABC = Lerp(AB, BC, t);
	float BCD = Lerp(BC, CD, t);
	float ABCD = Lerp(ABC, BCD, t);
	return ABCD;
}

float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t)
{
	float AB = Lerp(A, B, t);
	float BC = Lerp(B, C, t);
	float CD = Lerp(C, D, t);
	float DE = Lerp(D, E, t);
	float EF = Lerp(E, F, t);

	float ABC = Lerp(AB, BC, t);
	float BCD = Lerp(BC, CD, t);
	float CDE = Lerp(CD, DE, t);
	float DEF = Lerp(DE, EF, t);

	float ABCD = Lerp(ABC, BCD, t);
	float BCDE = Lerp(BCD, CDE, t);
	float CDEF = Lerp(CDE, DEF, t);

	float ABCDE = Lerp(ABCD, BCDE, t);
	float BCDEF = Lerp(BCDE, CDEF, t);

	float ABCDEF = Lerp(ABCDE, BCDEF, t);
	return ABCDEF;
}

float GetValueFromEasingFunction(EasingFunction easingFunction, float normalizedT)
{
	switch (easingFunction)
	{
	case LINEAR:
		return normalizedT;
	case SMOOTH_START_2:
		return SmoothStart2(normalizedT);
	case SMOOTH_START_3:
		return SmoothStart3(normalizedT);
	case SMOOTH_START_4:
		return SmoothStart4(normalizedT);
	case SMOOTH_START_5:
		return SmoothStart5(normalizedT);
	case SMOOTH_START_6:
		return SmoothStart6(normalizedT);
	case SMOOTH_STOP_2:
		return SmoothStop2(normalizedT);
	case SMOOTH_STOP_3:
		return SmoothStop3(normalizedT);
	case SMOOTH_STOP_4:
		return SmoothStop4(normalizedT);
	case SMOOTH_STOP_5:
		return SmoothStop5(normalizedT);
	case SMOOTH_STOP_6:
		return SmoothStop6(normalizedT);
	case SMOOTH_STEP_3:
		return SmoothStep3(normalizedT);
	case SMOOTH_STEP_5:
		return SmoothStep5(normalizedT);
	case HESITATE_3:
		return Hesitate3(normalizedT);
	case HESITATE_5:
		return Hesitate5(normalizedT);
	case SMOOTH_START_1_POINT_5:
		return Lerp(normalizedT, SmoothStart2(normalizedT), .5f);
	case SMOOTH_STOP_1_POINT_5:
		return Lerp(normalizedT, SmoothStop2(normalizedT), .5f);
	default:
		return normalizedT;
	}
}

float SmoothStart2(float t)
{
	return t*t;
}

float SmoothStart3(float t)
{
	return t*t*t;
}

float SmoothStart4(float t)
{
	return t*t*t*t;
}

float SmoothStart5(float t)
{
	return t*t*t*t*t;
}

float SmoothStart6(float t)
{
	return t*t*t*t*t*t;
}

float SmoothStop2(float t)
{
	float s = 1.f - t;
	return 1.f - (s*s);
}

float SmoothStop3(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s);
}

float SmoothStop4(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s * s);
}

float SmoothStop5(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s * s * s);
}

float SmoothStop6(float t)
{
	float s = 1.f - t;
	return 1.f - (s * s * s * s * s * s);
}

float SmoothStep3(float t)
{
	return Lerp(SmoothStart2(t), SmoothStop2(t), t);
}

float SmoothStep5(float t)
{
	return ComputeQuinticBezier1D(0, 0, 0, 1, 1, 1, t);
}

float Hesitate3(float t)
{
	return ComputeCubicBezier1D(0.f, 1.f, 0.f, 1.f, t);
}

float Hesitate5(float t)
{
	return ComputeQuinticBezier1D(0.f, 1.f, 0.f, 1.f, 0.f, 1.f, t);
}


float Clamp(float value, float min, float max)
{
	if (value > max)
		return max;
	if (value < min)
		return min;
	return value;
}

float Clamp(float value, FloatRange range)
{
	if (value > range.m_max)
	{
		return range.m_max;
	}
	if (value < range.m_min)
	{
		return range.m_min;
	}
	return value;
}

int ClampInt(int value, int min, int max)
{
	if (value > max)
		return max;
	if (value < min)
		return min;
	return value;
}

float Max(float a, float b)
{
	if (a > b)
		return a;
	return b;
}

float NormalizeByte(unsigned char byte)
{
	return static_cast<float>(byte) / 255.f;
}

unsigned char DenormalizeByte(float normalizedValue)
{
	//clamp in float in range 0 - 256
	float valueIn256Range = Clamp(normalizedValue * 256.f, 0.f, 256.f);
	//at the end of the spectrum return 1 
	if (valueIn256Range == 256.f)
	{
		return 255;
	}
	//otherwise round down to get even distribution
	return static_cast<unsigned char>(valueIn256Range);
}

float Min(float a, float b)
{
	if (a < b)
		return a;
	return b;
}

float ClampZeroToOne(float value)
{
	if (value > 1.f)
		return 1.f;
	if (value < 0.f)
		return 0.f;
	return value;
}

CubicBezierCurve2D::CubicBezierCurve2D(CubicHermiteCurve2D const& fromHermite)
{
	m_startPos = fromHermite.m_startPos;
	m_endPos = fromHermite.m_endPos;
	m_guidePos1 = m_startPos + (1.f/3.f * fromHermite.m_startVel);
	m_guidePos2 = m_endPos - (1.f/3.f * fromHermite.m_endVel);
}

CubicBezierCurve2D::CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos)
	: m_startPos(startPos)
	, m_guidePos1(guidePos1)
	, m_guidePos2(guidePos2)
	, m_endPos(endPos)
{

}

Vec2 CubicBezierCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	Vec2 AB = Vec2::Lerp(m_startPos, m_guidePos1, parametricZeroToOne);
	Vec2 BC = Vec2::Lerp(m_guidePos1, m_guidePos2, parametricZeroToOne);
	Vec2 CD = Vec2::Lerp(m_guidePos2, m_endPos, parametricZeroToOne);

	Vec2 ABC = Vec2::Lerp(AB, BC, parametricZeroToOne);
	Vec2 CDE = Vec2::Lerp(BC, CD, parametricZeroToOne);

	return Vec2::Lerp(ABC, CDE, parametricZeroToOne);
}

float CubicBezierCurve2D::GetApproximateLength(int numSubdivisions)
{
	float approximateLength = 0.f;
	float tStep = 1.f/ (float)numSubdivisions;
	for (int i = 0; i < numSubdivisions; i++)
	{
		float currT = (float)i / (float)numSubdivisions;
		float nextT = currT + tStep;

		Vec2 segmentStart = EvaluateAtParametric(currT);
		Vec2 segmentEnd = EvaluateAtParametric(nextT);

		approximateLength += GetDistance2D(segmentStart, segmentEnd);
	}

	return approximateLength;
}

Vec2 CubicBezierCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivisions)
{
	float distanceLeftToTravel = distanceAlongCurve;

	float tStep = 1.f / (float)numSubdivisions;
	for (int i = 0; i < numSubdivisions; i++)
	{
		float currT = (float)i / (float)numSubdivisions;
		float nextT = currT + tStep;

		Vec2 segmentStart = EvaluateAtParametric(currT);
		Vec2 segmentEnd = EvaluateAtParametric(nextT);

		float segmentDistance = GetDistance2D(segmentStart, segmentEnd);
		if (segmentDistance >= distanceLeftToTravel)
		{
			Vec2 directionAlongSegment = (segmentEnd - segmentStart).GetNormalized();
			Vec2 endLocation = segmentStart + directionAlongSegment * distanceLeftToTravel;
			return endLocation;
		}
		distanceLeftToTravel -= segmentDistance;
	}

	return EvaluateAtParametric(1.f);
}

CatmullRomSpline::CatmullRomSpline(std::vector<Vec2> const& positions)
{
	SetPositions(positions);
}

void CatmullRomSpline::SetPositions(std::vector<Vec2> const& positions)
{
	m_positions = positions;
	m_velocities.resize(m_positions.size());
	for (size_t i = 0; i < m_velocities.size(); i++)
	{
		if (i == 0 || i == m_velocities.size() - 1)
		{
			m_velocities[i] = Vec2::ZERO;
		}
		else
		{
			m_velocities[i] = (m_positions[i + 1] - m_positions[i - 1]) * .5f;
		}
	}
}

std::vector<Vec2> CatmullRomSpline::GetVelocities() const
{
	return m_velocities;
}

std::vector<Vec2> CatmullRomSpline::GetPositions() const
{
	return m_positions;
}

Vec2 CatmullRomSpline::EvaluateAtParametric(float parametricT) const
{
	int startingIdx = (int)floorf(parametricT);
	CubicHermiteCurve2D hermite(m_positions[startingIdx], m_velocities[startingIdx], m_positions[startingIdx + 1], m_velocities[startingIdx + 1]);
	return hermite.EvaluateAtParametric(parametricT - floorf(parametricT));
}

float CatmullRomSpline::GetApproximateLength(int numSubdivisions)
{
	float approximateLength = 0.f;
	for (int i = 0; i < (int)m_positions.size() - 1; i++)
	{
		CubicHermiteCurve2D hermite(m_positions[i], m_velocities[i], m_positions[i + 1], m_velocities[i + 1]);
		approximateLength += hermite.GetApproximateLength(numSubdivisions);
	}
	return approximateLength;
}

Vec2 CatmullRomSpline::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivions)
{
	float distanceLeft = distanceAlongCurve;
	for (int i = 0; i < (int)m_positions.size() - 1; i++)
	{
		CubicHermiteCurve2D hermite(m_positions[i], m_velocities[i], m_positions[i + 1], m_velocities[i + 1]);
		float currHermiteDistance = hermite.GetApproximateLength(numSubdivions);
		if (currHermiteDistance > distanceLeft)
		{
			return hermite.EvaluateAtApproximateDistance(distanceLeft, numSubdivions);
		}
		distanceLeft -= currHermiteDistance;
	}
	
	return m_positions[m_positions.size() - 1];
}

CubicHermiteCurve2D::CubicHermiteCurve2D(Vec2 const& startPos, Vec2 const& startVel, Vec2 const& endPos, Vec2 const& endVel)
	: m_startPos(startPos)
	, m_endPos(endPos)
	, m_startVel(startVel)
	, m_endVel(endVel)
{
}

CubicHermiteCurve2D::CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier)
{
	m_startVel = 3.f*(fromBezier.m_guidePos1 - fromBezier.m_startPos);
	m_endVel = 3.f*(fromBezier.m_endPos - fromBezier.m_guidePos2);
	m_startPos = fromBezier.m_startPos;
	m_endPos = fromBezier.m_endPos;
}

Vec2 CubicHermiteCurve2D::EvaluateAtParametric(float parametricZeroToOne) const
{
	return CubicBezierCurve2D(*this).EvaluateAtParametric(parametricZeroToOne);
}

float CubicHermiteCurve2D::GetApproximateLength(int numSubdivisions)
{
	return CubicBezierCurve2D(*this).GetApproximateLength(numSubdivisions);
}

Vec2 CubicHermiteCurve2D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivions)
{
	return CubicBezierCurve2D(*this).EvaluateAtApproximateDistance(distanceAlongCurve, numSubdivions);
}


ConvexPoly2D::ConvexPoly2D(std::vector<Vec2> const& positions)
	: m_vertexPositions(positions)
{

}

ConvexHull2D::ConvexHull2D(std::vector<Vec2> const& positions)
{
	for (int i = 0; i < (int)positions.size() - 1; i++)
	{
		Vec2 planeNormal = positions[i + 1] - positions[i];
		planeNormal = planeNormal.GetRotatedMinus90Degrees();
		planeNormal = planeNormal.GetNormalized();
		float distanceAlongPlaneNormal = DotProduct2D(positions[i], planeNormal);
		Plane2 plane(planeNormal, distanceAlongPlaneNormal);
		m_boundingPlanes.push_back(plane);
	}

	Vec2 planeNormal = positions[0] - positions[positions.size() - 1];
	planeNormal = planeNormal.GetRotatedMinus90Degrees();
	planeNormal = planeNormal.GetNormalized();
	float distanceAlongPlaneNormal = DotProduct2D(positions[0], planeNormal);
	Plane2 plane(planeNormal, distanceAlongPlaneNormal);
	m_boundingPlanes.push_back(plane);
}

ConvexHull2D::ConvexHull2D(std::vector<Plane2> const& planes)
	: m_boundingPlanes(planes)
{

}

CubicBezierCurve3D::CubicBezierCurve3D(Vec3 startPos, Vec3 guidePos1, Vec3 guidePos2, Vec3 endPos)
	: m_startPos(startPos)
	, m_guidePos1(guidePos1)
	, m_guidePos2(guidePos2)
	, m_endPos(endPos)
{
}

Vec3 CubicBezierCurve3D::EvaluateAtParametric(float parametricZeroToOne) const
{
	Vec3 AB = Vec3::Lerp(m_startPos, m_guidePos1, parametricZeroToOne);
	Vec3 BC = Vec3::Lerp(m_guidePos1, m_guidePos2, parametricZeroToOne);
	Vec3 CD = Vec3::Lerp(m_guidePos2, m_endPos, parametricZeroToOne);

	Vec3 ABC = Vec3::Lerp(AB, BC, parametricZeroToOne);
	Vec3 CDE = Vec3::Lerp(BC, CD, parametricZeroToOne);

	return Vec3::Lerp(ABC, CDE, parametricZeroToOne);
}

float CubicBezierCurve3D::GetApproximateLength(int numSubdivisions)
{
	float approximateLength = 0.f;
	float tStep = 1.f / (float)numSubdivisions;
	for (int i = 0; i < numSubdivisions; i++)
	{
		float currT = (float)i / (float)numSubdivisions;
		float nextT = currT + tStep;

		Vec3 segmentStart = EvaluateAtParametric(currT);
		Vec3 segmentEnd = EvaluateAtParametric(nextT);

		approximateLength += GetDistance3D(segmentStart, segmentEnd);
	}

	return approximateLength;
}

Vec3 CubicBezierCurve3D::EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivions)
{
	float distanceLeftToTravel = distanceAlongCurve;

	float tStep = 1.f / (float)numSubdivions;
	for (int i = 0; i < numSubdivions; i++)
	{
		float currT = (float)i / (float)numSubdivions;
		float nextT = currT + tStep;

		Vec3 segmentStart = EvaluateAtParametric(currT);
		Vec3 segmentEnd = EvaluateAtParametric(nextT);

		float segmentDistance = GetDistance3D(segmentStart, segmentEnd);
		if (segmentDistance >= distanceLeftToTravel)
		{
			Vec3 directionAlongSegment = (segmentEnd - segmentStart).GetNormalized();
			Vec3 endLocation = segmentStart + directionAlongSegment * distanceLeftToTravel;
			return endLocation;
		}
		distanceLeftToTravel -= segmentDistance;
	}

	return EvaluateAtParametric(1.f);
}
