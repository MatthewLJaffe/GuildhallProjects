#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"


class OBB3
{
public:
	OBB3(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& halfDims, Vec3 const& center);
	Vec3 m_iBasis;
	Vec3 m_jBasis;
	Vec3 m_kBasis;
	Vec3 m_halfDimensions;
	Vec3 m_center;
	std::vector<Vec3> GetCorners() const;
};