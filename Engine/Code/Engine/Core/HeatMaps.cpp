#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/VertexUtils.hpp"

TileHeatMap::TileHeatMap(IntVec2 const& dimensions)
	: m_dimensions(dimensions)
{
	int valuesSize = m_dimensions.x * m_dimensions.y;
	m_values = std::vector<float>(valuesSize, 0.f);
}

void TileHeatMap::SetAllValues(float defaultValue)
{
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			int idx = x + y * m_dimensions.x;
			m_values[idx] = defaultValue;
		}
	}
}

float TileHeatMap::GetHeatValue(IntVec2 const& coords) const
{
	return m_values[GetHeatIdxFromTileCoords(coords)];
}

void TileHeatMap::SetHeatValue(IntVec2 const& coords, float value)
{
	m_values[GetHeatIdxFromTileCoords(coords)] = value;
}

void TileHeatMap::AddToHeatValue(IntVec2 coords, float valueToAdd)
{
	m_values[GetHeatIdxFromTileCoords(coords)] += valueToAdd;
}

void TileHeatMap::AddVertsForDebugDraw(std::vector<Vertex_PCU>& verts, AABB2 bounds, FloatRange valueRange, Rgba8 lowColor, Rgba8 highColor, float specialValue, Rgba8 specialColor) const
{
	int vertsToReserve = m_dimensions.x * m_dimensions.y * 6;
	verts.reserve(vertsToReserve);
	float xStep = bounds.GetDimensions().x / static_cast<float>(m_dimensions.x);
	float yStep = bounds.GetDimensions().y / static_cast<float>(m_dimensions.y);
	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			Vec2 tileMins = Vec2((float)x * xStep, (float)y * yStep);
			Vec2 tileMaxs = Vec2(((float)x + 1.f) * xStep, ((float)y + 1.f) * yStep);
			AABB2 tileBounds(tileMins, tileMaxs);
			if (!bounds.IsAABB2Inside(tileBounds))
			{
				continue;
			}

			Rgba8 tileColor;
			float heatValue = GetHeatValue(IntVec2(x, y));
			if (heatValue == specialValue)
			{
				tileColor = specialColor;
			}
			else
			{
				float colorT = RangeMapClamped(heatValue, valueRange.m_min, valueRange.m_max, 0.f, 1.f);
				tileColor = LerpColor(lowColor, highColor, colorT);
			}
			AddVertsForAABB2D(verts, tileBounds, tileColor);
		}
	}
}

RaycastResult2D TileHeatMap::StepAndSampleRaycastVsSpecialHeat(Vec2 const& startPos, Vec2 const& normalizedDir, float distance, float specialHeat)
{
	constexpr float STEP_DISTANCE = .01f;
	Vec2 rayStep = normalizedDir * STEP_DISTANCE;
	Vec2 currPos = startPos;
	int steps = static_cast<int>(distance / STEP_DISTANCE);
	IntVec2 prevTileCoords = IntVec2((int)currPos.x, (int)currPos.y);
	RaycastResult2D raycastResult;
	for (int i = 0; i < steps; i++)
	{
		IntVec2 currTileCoords(RoundDownToInt(currPos.x), RoundDownToInt(currPos.y));
		if (m_values[GetHeatIdxFromTileCoords(currTileCoords)] == specialHeat)
		{
			Vec2 normal = Vec2((float)(prevTileCoords.x - currTileCoords.x), (float)(prevTileCoords.y - currTileCoords.y));
			raycastResult.m_impactPos = currPos;
			raycastResult.m_impactNormal = normal;
			raycastResult.m_impactDist = GetDistance2D(startPos, currPos);
			raycastResult.m_didImpact = true;
			return raycastResult;
		}
		currPos += rayStep;
		prevTileCoords = currTileCoords;
	}
	return raycastResult;
}

RaycastResult2D TileHeatMap::ImprovedRaycastVsSpecialHeat(Vec2 const& startPos, Vec2 const& rayFwdNormal, float rayLength, float specialHeat)
{
	RaycastResult2D result;
	result.m_rayStartPos = startPos;
	result.m_rayMaxLength = rayLength;
	result.m_rayFwdNormal = rayFwdNormal;
	//check if start pos is a hit
	IntVec2 startTileCoords(RoundDownToInt(startPos.x), RoundDownToInt(startPos.y));
	if (m_values[GetHeatIdxFromTileCoords(startTileCoords)] == specialHeat)
	{
		result.m_impactPos = startPos;
		result.m_impactDist = 0;
		result.m_didImpact = true;
		return result;
	}

	int xStepDir = rayFwdNormal.x > 0 ? 1 : -1;
	int yStepDir = rayFwdNormal.y > 0 ? 1 : -1;
	float distanceLeft = rayLength;
	Vec2 currPos = startPos;
	IntVec2 currTileCoords(RoundDownToInt(currPos.x), RoundDownToInt(currPos.y));

	while (distanceLeft > 0)
	{
		//figure out how long to travel along forward normal to reach next x and y
		int nextX = currTileCoords.x + (xStepDir + 1) / 2;
		float distToNextX = static_cast<float>(nextX) - currPos.x;
		float forwardLenToNextX = fabsf(distToNextX / rayFwdNormal.x);

		int nextY = currTileCoords.y + (yStepDir + 1) / 2;
		float distToNextY = static_cast<float>(nextY) - currPos.y;
		float forwardLenToNextY = fabsf(distToNextY / rayFwdNormal.y);

		Vec2 nextStepDisplacment;
		if (forwardLenToNextX < forwardLenToNextY)
		{
			nextStepDisplacment = forwardLenToNextX * rayFwdNormal;
			currTileCoords.x += xStepDir;
		}
		else
		{
			nextStepDisplacment = forwardLenToNextY * rayFwdNormal;
			currTileCoords.y += yStepDir;
		}
		distanceLeft -= nextStepDisplacment.GetLength();
		if (distanceLeft < 0)
		{
			break;
		}
		currPos += nextStepDisplacment;

		if (m_values[GetHeatIdxFromTileCoords(currTileCoords)] == specialHeat)
		{
			//crossing x
			if (forwardLenToNextX < forwardLenToNextY)
			{
				if (xStepDir > 0)
				{
					result.m_impactNormal = Vec2(-1, 0);
				}
				else
				{
					result.m_impactNormal = Vec2(1, 0);
				}
			}
			//crossing y
			else
			{
				if (yStepDir > 0)
				{
					result.m_impactNormal = Vec2(0, -1);
				}
				else
				{
					result.m_impactNormal = Vec2(0, 1);
				}
			}
			result.m_impactPos = currPos;
			result.m_impactDist = GetDistance2D(startPos, currPos);
			result.m_didImpact = true;
			return result;
		}
	}

	//miss
	result.m_impactDist = rayLength;
	return result;
}



int TileHeatMap::GetHeatIdxFromTileCoords(IntVec2 const& coords) const
{
	GUARANTEE_OR_DIE(coords.x >= 0 && coords.x < m_dimensions.x, "Tile coords out of range");
	GUARANTEE_OR_DIE(coords.y >= 0 && coords.y < m_dimensions.y, "Tile coords out of range");

	return coords.x + coords.y * m_dimensions.x;
}

IntVec2 TileHeatMap::GetDimensions() const
{
	return m_dimensions;
}
