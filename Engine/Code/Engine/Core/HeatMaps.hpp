#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"

struct Vertex_PCU;
class AABB2;
struct FloatRange;

class TileHeatMap
{
public:
	TileHeatMap(IntVec2 const& dimensions);
	void SetAllValues(float defaultValue);
	float GetHeatValue(IntVec2 const& coords) const;
	void SetHeatValue(IntVec2 const& coord, float value);
	void AddToHeatValue(IntVec2 coord, float valueToAdd);
	void AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 bounds, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor) const;
	RaycastResult2D StepAndSampleRaycastVsSpecialHeat(Vec2 const& startPos, Vec2 const& normalizedDir, float distance, float specialHeat);
	RaycastResult2D ImprovedRaycastVsSpecialHeat(Vec2 const& startPos, Vec2 const& normalizedDir, float distance, float specialHeat);

	IntVec2 GetDimensions() const;
private:
	int GetHeatIdxFromTileCoords(IntVec2 const& coords) const;
	IntVec2 m_dimensions;
	std::vector<float> m_values;
};