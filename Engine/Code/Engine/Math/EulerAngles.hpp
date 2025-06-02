#pragma once


struct Vec3;
struct Mat44;

struct EulerAngles
{
	static const EulerAngles IDENTITY;

	EulerAngles() = default;
	EulerAngles(float yaw, float pitch, float roll);

	float m_yaw = 0.f;
	float m_pitch = 0.f;
	float m_roll = 0.f;

	bool operator==(EulerAngles const& compare) const;			//EulerAngles == EulerAngles
	EulerAngles operator+(EulerAngles const& add) const;			//EulerAngles + EulerAngles
	bool operator!=(EulerAngles const& compare) const;			//EulerAngles != EulerAngles
	void GetAsVectors_IFwd_JLeft_KUp(Vec3& iFwd, Vec3& jLeft, Vec3& kUp) const;
	Mat44 GetAsMatrix_IFwd_JLeft_KUp() const;
	Vec3 GetIFwd() const;
	Vec3 GetJLeft() const;
	Vec3 GetKUp() const;
	Vec3 GetVectorRotated(Vec3 vectorToRotate);
	void SetFromText(char const* text);
	void GetFromFloats(float* out_floats);
	void SetFromFloats(float const* floats);

}; 
