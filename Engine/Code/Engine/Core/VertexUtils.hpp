#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Mat44.hpp"
#include <math.h>

class CubicBezierCurve2D;
class CubicHermiteCurve2D;
class CatmullRomSpline;

typedef float(*EasingFunctionPtr)(float);

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY);
void TransformVertexArrayXY3D(size_t numVerts, Vertex_PCU* m_localVerts, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translationXY);
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
void AddVertsForOrientedSector2D(std::vector<Vertex_PCU>& verts, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius, Rgba8 const& color);
void AddVertsForDisc2D(std::vector<Vertex_PCU>& m_localVerts, Vec2 const& centerPos, float radius, Rgba8 const& color, int numSides = 32, float heightOffsetForVaporum = 0.f);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& centerPos, float radius, float thickness, Rgba8 const& color, int numSides = 32, float heightOffsetForVaporum = 0.f);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, Vec2 const& mins, Vec2 const& maxs, Rgba8 const& color, Vec2 const& uvMins = Vec2::ZERO, Vec2 const& uvMaxs = Vec2::ONE);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, Vec2 const& uvMins = Vec2::ZERO, Vec2 const& uvMaxs = Vec2::ONE, float sortOrder = 0);
void AddVertsForOOB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color);
void AddVertsForLine2D(std::vector<Vertex_PCU>& verts, Vec2 const& pointA, Vec2 const& pointB, float thickness, Rgba8 const& color);
void AddVertsForArrow2D( std::vector<Vertex_PCU>& verts, Vec2 tailPos, Vec2 tipPos, float arrowSize, float lineThickness, Rgba8 const& color );
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, 
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, 
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, 
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, const AABB3& bounds, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, const Vec3& center, float radius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16);
void AddVertsForSphere3D(std::vector<Vertex_PCUTBN>& verts, const Vec3& center, float radius, const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE, int numSlices = 32, int numStacks = 16);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, const Mat44& transform);
void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, const Mat44& transform, bool transformNormals = false);
AABB2 GetVertexBounds2D(const std::vector<Vertex_PCU> verts);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius,
	const Rgba8 color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE,
	int numSlices = 16);
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius,
	const Rgba8& color = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE,
	int numSlices = 8);

void AddVertsForEasingFunction(std::vector<Vertex_PCU>& verts, EasingFunctionPtr easingFunctionPtr, Vec2 const& startPos, Vec2 const& endPos, float lineThickness, 
	Rgba8 color = Rgba8::WHITE, int numSubdivisions = 64);

void AddVertsForCubicBezier(std::vector<Vertex_PCU>& verts, CubicBezierCurve2D const& cubicBezier, float lineThickness, Rgba8 color = Rgba8::WHITE, int numSubdivisions = 64);
void AddVertsForCubicHermite(std::vector<Vertex_PCU>& verts, CubicHermiteCurve2D const& cubicHermite, float lineThickness, Rgba8 color = Rgba8::WHITE, int numSubdivisions = 64);
void AddVertsForCatmullRomSpline(std::vector<Vertex_PCU>& verts, CatmullRomSpline const& catmulRomSpline, float lineThickness, Rgba8 color = Rgba8::WHITE, int numSubdivisions = 64);

void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts,
	const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft,
	const Rgba8& color = Rgba8::WHITE, const AABB2& UVs = AABB2::ZERO_TO_ONE);

void AddVertsForArrow3D(std::vector<Vertex_PCU>& verts, const Vec3& start, const Vec3& end, float radius, Rgba8 color = Rgba8::WHITE);
void AddVertsForLine3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius,
	const Rgba8 startColor = Rgba8::WHITE,
	const Rgba8 endColor = Rgba8::WHITE,
	const AABB2& UVs = AABB2::ZERO_TO_ONE,
	int numSlices = 16);

void AddVertsForBoxLine3D(std::vector<Vertex_PCU>& verts,
	const Vec3& start, const Vec3& end, float radius,
	const Rgba8 color = Rgba8::WHITE);


void AddVertsForBasis3D(std::vector<Vertex_PCU>& verts, Mat44 const& basis);
void AddVertsForOBB3(std::vector<Vertex_PCU>& verts, OBB3 obb3, Rgba8 const& color = Rgba8::WHITE);
void CalculateTangentSpaceBasisVectors(
	std::vector<Vertex_PCUTBN>& vertexes,
	std::vector<unsigned int>& indexes,
	bool computeNormals = true,
	bool computeTangents = true
);

void AddVertsForBoxEdges(std::vector<Vertex_PCU>& verts, AABB3 const& boxDimensions, float lineWidth, Rgba8 const& color = Rgba8::WHITE);

void AddVertsForPyramid3D(std::vector<Vertex_PCUTBN>& verts, Vec3 const& basePosition, float width, float height, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForCubeMapSkyBox(std::vector<Vertex_PCU>& verts, Vec3 const& centerPositoin, float sideLength, Rgba8 const& color = Rgba8::WHITE);