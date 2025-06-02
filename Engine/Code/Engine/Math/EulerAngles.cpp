#include "EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/EngineCommon.hpp"

EulerAngles const EulerAngles::IDENTITY = EulerAngles(0.f, 0.f, 0.f);


EulerAngles::EulerAngles(float inputYaw, float inputPitch, float inputRoll)
	: m_yaw(inputYaw)
	, m_pitch(inputPitch)
	, m_roll(inputRoll)
{

}

Vec3 EulerAngles::GetVectorRotated(Vec3 vectorToRotate)
{
	return GetAsMatrix_IFwd_JLeft_KUp().TransformVectorQuantity3D(vectorToRotate);
}

bool EulerAngles::operator==(EulerAngles const& compare) const
{
	return m_yaw == compare.m_yaw && m_pitch == compare.m_pitch && m_roll == compare.m_roll;
}

bool EulerAngles::operator!=(EulerAngles const& compare) const
{
	return m_yaw != compare.m_yaw || m_pitch != compare.m_pitch || m_roll != compare.m_roll;
}

EulerAngles EulerAngles::operator+(EulerAngles const& add) const
{
	return EulerAngles(m_yaw + add.m_yaw, m_pitch + add.m_pitch, m_roll + add.m_roll);
}


void EulerAngles::GetAsVectors_IFwd_JLeft_KUp(Vec3& iFwd, Vec3& jLeft, Vec3& kUp) const
{
	Mat44 rotation;
	rotation.AppendZRotation(m_yaw);
	rotation.AppendYRotation(m_pitch);
	rotation.AppendXRotation(m_roll);

	iFwd = rotation.GetIBasis3D();
	jLeft = rotation.GetJBasis3D();
	kUp = rotation.GetKBasis3D();
}

Mat44 EulerAngles::GetAsMatrix_IFwd_JLeft_KUp() const
{
	Mat44 rotation;
	rotation.AppendZRotation(m_yaw);
	rotation.AppendYRotation(m_pitch);
	rotation.AppendXRotation(m_roll);
	return rotation;
}

Vec3 EulerAngles::GetIFwd() const
{
	Mat44 rotation;
	rotation.AppendZRotation(m_yaw);
	rotation.AppendYRotation(m_pitch);
	rotation.AppendXRotation(m_roll);

	return rotation.GetIBasis3D();
}

Vec3 EulerAngles::GetJLeft() const
{
	Mat44 rotation;
	rotation.AppendZRotation(m_yaw);
	rotation.AppendYRotation(m_pitch);
	rotation.AppendXRotation(m_roll);

	return rotation.GetJBasis3D();
}

Vec3 EulerAngles::GetKUp() const
{
	Mat44 rotation;
	rotation.AppendZRotation(m_yaw);
	rotation.AppendYRotation(m_pitch);
	rotation.AppendXRotation(m_roll);

	return rotation.GetKBasis3D();
}

void EulerAngles::SetFromText(char const* text)
{
	Strings values = SplitStringOnDelimiter(text, ',');
	GUARANTEE_OR_DIE(values.size() == 3, "Parsing EulerAngles failed to get yaw pitch roll values");
	this->m_yaw = (float)atof(values[0].c_str());
	this->m_pitch = (float)atof(values[1].c_str());
	this->m_roll = (float)atof(values[2].c_str());
}

void EulerAngles::GetFromFloats(float* out_floats)
{
	out_floats[0] = m_yaw;
	out_floats[1] = m_pitch;
	out_floats[2] = m_roll;
}

void EulerAngles::SetFromFloats(float const* floats)
{
	m_yaw = floats[0];
	m_pitch = floats[1];
	m_roll = floats[2];
}
