#include "Engine/Math/OBB3.hpp"

OBB3::OBB3(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& halfDims, Vec3 const& center)
	: m_iBasis(iBasis)
	, m_jBasis(jBasis)
	, m_kBasis(kBasis)
	, m_halfDimensions(halfDims)
	, m_center(center)
{
}

std::vector<Vec3> OBB3::GetCorners() const
{
	std::vector<Vec3> corners;
	Vec3 backLeftBottom = m_center - (m_halfDimensions.x*m_iBasis) + (m_halfDimensions.y * m_jBasis)- (m_halfDimensions.z * m_kBasis);
	corners.push_back(backLeftBottom);

	Vec3 backRightBottom = m_center - (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) - (m_halfDimensions.z * m_kBasis);
	corners.push_back(backRightBottom);

	Vec3 frontRightBottom = m_center + (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) - (m_halfDimensions.z * m_kBasis);
	corners.push_back(frontRightBottom);

	Vec3 frontLeftBottom = m_center + (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * m_jBasis) - (m_halfDimensions.z * m_kBasis);
	corners.push_back(frontLeftBottom);

	Vec3 backLeftTop = m_center - (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);
	corners.push_back(backLeftTop);

	Vec3 backRightTop = m_center - (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);
	corners.push_back(backRightTop);

	Vec3 frontRightTop = m_center + (m_halfDimensions.x * m_iBasis) - (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);
	corners.push_back(frontRightTop);

	Vec3 frontLeftTop = m_center + (m_halfDimensions.x * m_iBasis) + (m_halfDimensions.y * m_jBasis) + (m_halfDimensions.z * m_kBasis);
	corners.push_back(frontLeftTop);

	return corners;
}
