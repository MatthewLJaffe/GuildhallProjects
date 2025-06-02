#pragma once
#include <math.h>
#include <Vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include <Engine/Math/AABB3.hpp>
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Plane2.hpp"

struct Vec2;
struct Vec3;
struct Vec4;
struct IntVec2;
class AABB2;
class OBB2;
struct Plane3;
class OBB3;

struct RaycastResult2D
{
	// Basic raycast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec2	m_impactPos;
	Vec2	m_impactNormal;

	// Original raycast information (optional)
	Vec2	m_rayFwdNormal;
	Vec2	m_rayStartPos;
	float	m_rayMaxLength = 1.f;
};

struct RaycastResult3D
{
	// Basic raycast result information (required)
	bool	m_didImpact = false;
	float	m_impactDist = 0.f;
	Vec3	m_impactPos;
	Vec3	m_impactNormal;

	// Original raycast information (optional)
	Vec3	m_rayFwdNormal;
	Vec3	m_rayStartPos;
	float	m_rayMaxLength = 1.f;
};

enum class BillboardType
{
	NONE = -1,
	WORLD_UP_FACING,
	WORLD_UP_OPPOSING,
	FULL_FACING,
	FULL_OPPOSING,
	COUNT
};

enum EasingFunction
{
	LINEAR = 0,
	SMOOTH_START_2 = 1,
	SMOOTH_START_3 = 2,
	SMOOTH_START_4 = 3,
	SMOOTH_START_5 = 4,
	SMOOTH_START_6 = 5,
	SMOOTH_STOP_2 = 6,
	SMOOTH_STOP_3 = 7,
	SMOOTH_STOP_4 = 8,
	SMOOTH_STOP_5 = 9,
	SMOOTH_STOP_6 = 10,
	SMOOTH_STEP_3 = 11,
	SMOOTH_STEP_5 = 12,
	HESITATE_3 = 13,
	HESITATE_5 = 14,
	SMOOTH_START_1_POINT_5 = 15,
	SMOOTH_STOP_1_POINT_5 = 16
};

class CubicHermiteCurve2D;

class CubicBezierCurve2D
{
public:
	CubicBezierCurve2D() = default;
	explicit CubicBezierCurve2D(CubicHermiteCurve2D const& fromHermite);
	CubicBezierCurve2D( Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos );
	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength( int numSubdivisions=64 );
	Vec2 EvaluateAtApproximateDistance( float distanceAlongCurve, int numSubdivions=64 );
public:
	Vec2 m_startPos;
	Vec2 m_guidePos1;
	Vec2 m_guidePos2;
	Vec2 m_endPos;
};

class CubicBezierCurve3D
{
public:
	CubicBezierCurve3D() = default;
	CubicBezierCurve3D(Vec3 startPos, Vec3 guidePos1, Vec3 guidePos2, Vec3 endPos);
	Vec3 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64);
	Vec3 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivions = 64);
public:
	Vec3 m_startPos;
	Vec3 m_guidePos1;
	Vec3 m_guidePos2;
	Vec3 m_endPos;
};


class CubicHermiteCurve2D
{
public:
	CubicHermiteCurve2D(Vec2 const& startPos, Vec2 const& startVel, Vec2 const& endPos, Vec2 const& endVel);
	explicit CubicHermiteCurve2D(CubicBezierCurve2D const& fromBezier);
	Vec2 EvaluateAtParametric(float parametricZeroToOne) const;
	float GetApproximateLength(int numSubdivisions = 64);
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivions = 64);
	Vec2 m_startPos;
	Vec2 m_startVel;
	Vec2 m_endPos;
	Vec2 m_endVel;
};

class CatmullRomSpline
{
public:
	CatmullRomSpline() = default;
	CatmullRomSpline(std::vector<Vec2> const& positions);
	void SetPositions(std::vector<Vec2> const& positions);
	std::vector<Vec2> GetVelocities() const;
	std::vector<Vec2> GetPositions() const;
	Vec2 EvaluateAtParametric(float parametricT) const;
	float GetApproximateLength(int numSubdivisions = 64);
	Vec2 EvaluateAtApproximateDistance(float distanceAlongCurve, int numSubdivions = 64);
private:
	std::vector<Vec2> m_velocities;
	std::vector<Vec2> m_positions;
};



class ConvexPoly2D
{
public:
	ConvexPoly2D(std::vector<Vec2> const& positions);
	//in positive theta order
	std::vector <Vec2> m_vertexPositions;
};

class ConvexHull2D
{
public:
	ConvexHull2D() = default;
	ConvexHull2D(std::vector<Vec2> const& positions);
	ConvexHull2D(std::vector<Plane2> const& planes);
	std::vector<Plane2> m_boundingPlanes;
};

constexpr float PI = 3.14159265f;

//Angle utilities
float ConvertDegreesToRadians( float degrees );
float ConvertRadiansToDegrees ( float radians );
float CosDegrees( float degrees );
float SinDegrees( float degrees );
float TanDegrees ( float degrees );
float Atan2Degrees( float y, float x );
float GetShortestAngularDispDegrees(float currentDegrees, float goalDegrees);
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);
//Get turned towards 3d around axis?
float GetAngleDegreesBetweenVectors2D( Vec2 const& a, Vec2 const& b );
float GetAngleDegreesBetweenVectors3D(Vec3 const& a, Vec3 const& b);

float GetUnsignedAngleBetweenVectors3D(Vec3 const& a, Vec3 const& b);

// Basic 2D and 3D utilities
float GetDistance2D( Vec2 const& positionA, Vec2 const& positionB );
float GetDistanceSquared2D( Vec2 const& positionA, Vec2 const& positionB);
float GetDistance3D( Vec3 const& positionA, Vec3 const& positionB );
float GetDistanceSquared3D( Vec3 const& positionA, Vec3 const& positionB );
float GetDistanceXY3D( Vec3 const& positionA, Vec3 const& positionB );
float GetDistanceXYSquared3D( Vec3 const& positionA, Vec3 const& positionB );
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float GetProjectedLength2D( Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto );
Vec2 const GetProjectedOnto2D( Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto );
Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto);
Vec2 const GetNearestPointOnDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
Vec2 const GetNearestPointOnRing2D(Vec2 const& point, Vec2 const& ringCenter, float ringRadius);
Vec2 const GetNearestPointOnAABB2D( Vec2 const& point, AABB2 const& box );
Vec2 const GetNearestPointOnOBB2D(Vec2 const& point, OBB2 const& box);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& point, Vec2 const& lineStart, Vec2 const& lineEnd);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& point, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);
float const GetDistanceFromLineSegment2D(Vec2 const& point, Vec2 const& lineStart, Vec2 const& lineEnd);
float const GetDistanceFromLineSegment3D(Vec3 const& point, Vec3 const& lineStart, Vec3 const& lineEnd);
float const GetDistanceSquaredFromLineSegment3D(Vec3 const& point, Vec3 const& lineStart, Vec3 const& lineEnd);
Vec3 const GetNearestPointOnSphere3D(Vec3 const& point, Vec3 const& sphereCenter, float sphereRadius);
Vec3 const GetNearestPointOnCylinder3D(Vec3 const& point, Vec2 const& xyCenter, float radius, FloatRange zMinMax);
Vec3 const GetNearestPointOnAABB3D(Vec3 const& point, AABB3 const& box);
Vec3 const GetNearestPointOnOBB3(Vec3 const& point, OBB3 const& box);
Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& targetMatrix, const Vec3& billboardPosition, const Vec2& billboardScale = Vec2::ONE);
Mat44 GetRotationMatrixFromIBasisKUp(Vec3 const& iBasis);

//Geometric queries
//2D
bool IsPointInsideDisc2D( Vec2 const& point, Vec2 const& discCenter, float discRadius );
bool IsPointInsideAABB2D( Vec2 const& point, AABB2 const& box);
bool IsPointInsideOrientedSector2D( Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius );
bool IsPointInsideDirectedSector2D (Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox);
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool IsPointInsideConvexHull2D(Vec2 const& point, ConvexHull2D const& convexHull);
bool DoDiscsOverlap( Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB );
bool DoAABBsOverlap2D(AABB2 const& box1, AABB2 const& box2);
bool DoDiscAndAABBOverlap2D(Vec2 const& discCenter, float discRadius, AABB2 const& aabb2);
bool DoDiscAndOBBOverlap2D(Vec2 const& discCenter, float discRadius, OBB2 const& obb2);
bool PushDiscOutOfFixedPoint2D( Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedPoint );
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool PushDiscOutOfFixedAABB2D(Vec2& discCenter, float discRadius, AABB2 const& fixedBox);
bool BounceMobileDiscOffFixedPoint2D(Vec2& discCenter, Vec2& discVelocity, float discRadius, Vec2 const& fixedPoint, float elasticity = 1.f);
bool BounceMobileDiscsOffEachOther2D(Vec2& posA, Vec2& velA, float radiusA, Vec2& posB, Vec2& velB, float radiusB, float elasticity = 1.f);

//3D
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);
bool DoAABBsOverlap3D(AABB3 const& first, AABB3 const& second);
bool DoSpheresOverlap3D(Vec3 const& firstPos, float firstRadius, Vec3 const& secondPos, float secondRadius);
bool DoZCylindersOverlap(Vec2 const& firstXYCenter, float firstRadius, FloatRange firstZMinMax, Vec2 const& secondXYCenter, float secondRadius, FloatRange secondZMinMax);
bool DoSphereAndAABBOverlap3D(Vec3 const& sphereCenter, float sphereRadius, AABB3 const& box);
bool DoZCylinderAndAABBOverlap3D(Vec2 const& cylinderXYCenter, float cylinderRadius, FloatRange cylinderZMinMax, AABB3 const& box);
bool DoZCylinderAndSphereOverlap3D(Vec2 const& cylinderXYCenter, float cylinderRadius, FloatRange cylinderZMinMax, Vec3 const& sphereCenter, float sphereRadius);
bool DoPlaneAndOBB3Overlap3D(Plane3 const& plane, OBB3 const& obb3);
bool IsPointInsideSphere3D(Vec3 const& point,  Vec3 const& sphereCenter, float sphereRadius);
bool IsPointInsizeZCylinder3D(Vec3 const& point, Vec2 const&  cylinderXYCenter, float cylinderRadius, FloatRange cylinderZMinMax);
bool IsPointInsideOBB3(Vec3 const& point, OBB3 const& obb3);
bool IsPointInsideCapsule3D(Vec3 const& point, Vec3 const& boneStart, Vec3 const& boneEnd, float radius);
void CreateBasisVectorsFromIBasis(Vec3 const& iBasis, Vec3& outJbasis, Vec3& outKBasis);
bool PushSpheresOutOfEachOther3D(Vec3& aCenter, float aRadius, Vec3& bCenter, float bRadius);
bool PushSphereOutOfFixedSphere3D(Vec3 const& fixedSphereCenter, float fixedSphereRadius, Vec3& mobileSphereCenter, float mobileSphereRadius);

//Transform utilities
void TransformPosition2D( Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation );
void TransformPosition2D( Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation );
void TransformPositionXY3D( Vec3& positionToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY );
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);

//clamp and lerp
float Clamp(float value, float min, float max);
float Clamp(float value, FloatRange range);
int ClampInt(int value, int min, int max);
float ClampZeroToOne(float value);
float Lerp(float a, float b, float t);
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd);
float GetFractionWithinRange(float value, FloatRange range);

float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
int RoundDownToInt(float value);
float Min(float a, float b);
float Max(float a, float b);
float NormalizeByte(unsigned char byte);
unsigned char DenormalizeByte(float normalizedValue);

//dot and cross
float DotProduct2D(Vec2 const& a, Vec2 const& b);
float DotProduct3D(Vec3 const& a, Vec3 const& b);
float DotProduct4D(Vec4 const& a, Vec4 const& b);
float CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b);

//raycast
//2D
RaycastResult2D RaycastVsDisc2D( Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 discCenter, float discRadius );
RaycastResult2D RaycastVsLineSegment2D( Vec2 startPos, Vec2 fwdNormal, float maxDist, Vec2 lineStart, Vec2 lineEnd );
RaycastResult2D RaycastVsAABB2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, AABB2 const& aabb2);
RaycastResult2D RaycastVSConvexHull2D(Vec2 startPos, Vec2 fwdNormal, float maxDist, ConvexHull2D const& convexHull);
RaycastResult2D RaycastVsPlane2D(Vec2 const& rayStart, Vec2 const& rayForwardNormal, float rayLength, Plane2 const& plane);

RaycastResult3D RaycastVsAABB3D( Vec3 const& rayStart, Vec3 const& rayForwardnormal, float rayLength, AABB3 const& box);
RaycastResult3D RaycastVsSphere3D(Vec3 const& rayStart, Vec3 const& rayForwardnormal, float rayLength, Vec3 const& spherePos, float radius);
RaycastResult3D RaycastVsCylinderZ3D( Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayLength, Vec2 const& centerXY, float radiusXY, FloatRange zMinmax);
RaycastResult3D RaycastVsPlanes3D(Vec3 const& rayStart, Vec3 const& rayForwardNormal, float rayLength, Plane3 const& plane);
RaycastResult3D RaycastVsOBB3(Vec3 const& rayStart, Vec3 const& rayForwardnormal, float rayLength, OBB3 const& obb3);

//curves and splines
float ComputeCubicBezier1D(float a, float b, float c, float d, float t);
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float t);
float GetValueFromEasingFunction(EasingFunction easingFunction, float normalizedT);

float SmoothStart2(float t);
float SmoothStart3(float t);
float SmoothStart4(float t);
float SmoothStart5(float t);
float SmoothStart6(float t);

float SmoothStop2(float t);
float SmoothStop3(float t);
float SmoothStop4(float t);
float SmoothStop5(float t);
float SmoothStop6(float t);

float SmoothStep3(float t);
float SmoothStep5(float t);

float Hesitate3(float t);
float Hesitate5(float t);