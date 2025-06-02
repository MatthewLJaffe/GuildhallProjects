#include "Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"

Mat44::Mat44()
{
	m_values[Ix] = 1;
	m_values[Iy] = 0;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = 0;
	m_values[Jy] = 1;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

	m_values[Kx] = 0;
	m_values[Ky] = 0;
	m_values[Kz] = 1;
	m_values[Kw] = 0;

	m_values[Tx] = 0;
	m_values[Ty] = 0;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}

bool Mat44::operator==(const Mat44& compare) const
{
	for (int i = 0; i < 15; i++)
	{
		if (m_values[i] != compare.m_values[i])
		{
			return false;
		}
	}
	return true;
}

Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

	m_values[Kx] = 0;
	m_values[Ky] = 0;
	m_values[Kz] = 1;
	m_values[Kw] = 0;

	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}

Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0;

	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
	m_values[Tw] = 1;
}

Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

Mat44::Mat44(const float values[16])
{
	for (int i = 0; i < 16; i++)
	{
		m_values[i] = values[i];
	}
}

Mat44 const Mat44::CreateTranslation2D(Vec2 const& translationXY)
{
	return Mat44(Vec2::RIGHT, Vec2::UP, translationXY);
}

Mat44 const Mat44::CreateTranslation3D(Vec3 const& translationXYZ)
{
	return Mat44(Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1), translationXYZ);
}

Mat44 const Mat44::CreateUniformScale2D(float uniformScaleXY)
{
	return Mat44(Vec2(uniformScaleXY, 0), Vec2(0, uniformScaleXY), Vec2::ZERO);
}

Mat44 const Mat44::CreateUniformScale3D(float uniformScaleXYZ)
{
	return Mat44(Vec3(uniformScaleXYZ, 0, 0), Vec3(0, uniformScaleXYZ, 0), Vec3(0, 0, uniformScaleXYZ), Vec3(0, 0, 0));
}

Mat44 const Mat44::CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	return Mat44(Vec2(nonUniformScaleXY.x, 0), Vec2(0, nonUniformScaleXY.y), Vec2::ZERO);
}

Mat44 const Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	return Mat44(Vec3(nonUniformScaleXYZ.x, 0, 0), Vec3(0, nonUniformScaleXYZ.y, 0), Vec3(0, 0, nonUniformScaleXYZ.z), Vec3(0, 0, 0));
}

Mat44 const Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	Vec2 iBasis = Vec2(CosDegrees(rotationDegreesAboutZ), SinDegrees(rotationDegreesAboutZ));
	Vec2 jBasis = iBasis.GetRotated90Degrees();

	return Mat44(iBasis, jBasis, Vec2::ZERO);
}

Mat44 const Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	Vec3 iBasis(CosDegrees(rotationDegreesAboutY), 0.f, -SinDegrees(rotationDegreesAboutY));
	Vec3 jBasis(0.f, 1.f, 0.f);
	Vec3 kBasis(-iBasis.z, 0.f, iBasis.x);

	return Mat44(iBasis, jBasis, kBasis, Vec3(0,0,0));
}

Mat44 const Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	Vec3 iBasis(1, 0, 0);
	Vec3 jBasis(0, CosDegrees(rotationDegreesAboutX), SinDegrees(rotationDegreesAboutX));
	Vec3 kBasis(0, -jBasis.z, jBasis.y);

	return Mat44(iBasis, jBasis, kBasis, Vec3(0, 0, 0));
}

Mat44 const Mat44::CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	float scaleX = 2.f / (right - left);
	float translationX = (left + right) / (left - right);
	float scaleY = 2.f / (top - bottom);
	float translationY = (bottom + top) / (bottom - top);
	float scaleZ = 1.f / (zFar - zNear);
	float translationZ = -zNear / (zFar - zNear);

	Mat44 orthoProjectionMatrix;

	orthoProjectionMatrix.m_values[Ix] = scaleX;
	orthoProjectionMatrix.m_values[Tx] = translationX;

	orthoProjectionMatrix.m_values[Jy] = scaleY;
	orthoProjectionMatrix.m_values[Ty] = translationY;

	orthoProjectionMatrix.m_values[Kz] = scaleZ;
	orthoProjectionMatrix.m_values[Tz] = translationZ;

	return orthoProjectionMatrix;
}

Mat44 const Mat44::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat44 perspectiveProjection;
	perspectiveProjection.m_values[Jy] = CosDegrees(fovYDegrees * 0.5f) / SinDegrees(fovYDegrees * 0.5f);  // equals 1 if vertical Field of View is 90
	perspectiveProjection.m_values[Ix] = perspectiveProjection.m_values[Jy] / aspect; // equals scaleY if screen is square (aspect=1.0); equals 1 if square screen and FOV 90
	perspectiveProjection.m_values[Kz] = zFar / (zFar - zNear);
	perspectiveProjection.m_values[Kw] = 1.0f; // this puts Z into the W component (in preparation for the hardware w-divide)
	perspectiveProjection.m_values[Tz] = (zNear * zFar) / (zNear - zFar);
	perspectiveProjection.m_values[Tw] = 0.0f;  // Otherwise we would be putting Z+1 (instead of Z) into the W component
	return perspectiveProjection;
}

Mat44 const Mat44::CreateInversePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	/*
	//row reduction attempt 1
	Mat44 perspectiveProjection;
	perspectiveProjection.m_values[Ix] = aspect / (CosDegrees(fovYDegrees * 0.5f) / SinDegrees(fovYDegrees * 0.5f));
	perspectiveProjection.m_values[Jy] = SinDegrees(fovYDegrees * 0.5f) / CosDegrees(fovYDegrees * 0.5f);
	perspectiveProjection.m_values[Kz] = 0.f;
	perspectiveProjection.m_values[Kw] = (zNear - zFar)/(zNear*zFar); // this puts Z into the W component (in preparation for the hardware w-divide)
	perspectiveProjection.m_values[Tz] = 1.f;
	perspectiveProjection.m_values[Tw] = 1.f / zNear;  // Otherwise we would be putting Z+1 (instead of Z) into the W component
	return perspectiveProjection;
	*/

	//row reduction of matrix in dx coordinate system
	Mat44 perspectiveProjection;
	perspectiveProjection.m_values[Ix] = 0.f;
	perspectiveProjection.m_values[Iy] = -aspect / (CosDegrees(fovYDegrees * 0.5f) / SinDegrees(fovYDegrees * 0.5f));
	perspectiveProjection.m_values[Jy] = 0.f;
	perspectiveProjection.m_values[Jz] = (SinDegrees(fovYDegrees * 0.5f) / CosDegrees(fovYDegrees * 0.5f));
	perspectiveProjection.m_values[Kz] = 0.f;
	perspectiveProjection.m_values[Kw] = (zNear - zFar) / (zNear*zFar);
	perspectiveProjection.m_values[Tx] = 1.f;
	perspectiveProjection.m_values[Tw] = 1.f / zNear;
	return perspectiveProjection;
}

Mat44 const Mat44::CreateAxisAngleRotation(Vec3 const& axis, float angle)
{
	Mat44 rotationMatrix;
	rotationMatrix.m_values[Ix] = CosDegrees(angle) + (axis.x * axis.x) * (1.f - CosDegrees(angle));
	rotationMatrix.m_values[Iy] = axis.x*axis.y*(1 - CosDegrees(angle)) - axis.z*SinDegrees(angle);
	rotationMatrix.m_values[Iz] = axis.x*axis.z*(1 - CosDegrees(angle)) + axis.y*SinDegrees(angle);

	rotationMatrix.m_values[Jx] = axis.y*axis.x*(1.f - CosDegrees(angle)) + axis.z*SinDegrees(angle);
	rotationMatrix.m_values[Jy] = CosDegrees(angle) + axis.y*axis.y * (1.f - CosDegrees(angle));
	rotationMatrix.m_values[Jz] = axis.y*axis.z * (1 - CosDegrees(angle)) - axis.x*SinDegrees(angle);

	rotationMatrix.m_values[Kx] = axis.z*axis.x*(1.f - CosDegrees(angle)) - axis.y*SinDegrees(angle);
	rotationMatrix.m_values[Ky] = axis.z*axis.y*(1.f - CosDegrees(angle)) + axis.x*SinDegrees(angle);
	rotationMatrix.m_values[Kz] = CosDegrees(angle) + axis.z*axis.z*(1.f - CosDegrees(angle));

	return rotationMatrix;
}

Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
	float transformedX = m_values[Ix] * vectorQuantityXY.x + m_values[Jx] * vectorQuantityXY.y;
	float transformedY = m_values[Iy] * vectorQuantityXY.x + m_values[Jy] * vectorQuantityXY.y;

	return Vec2(transformedX, transformedY);
}

Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	float transformedX = m_values[Ix]*vectorQuantityXYZ.x + m_values[Jx]*vectorQuantityXYZ.y + m_values[Kx]*vectorQuantityXYZ.z;
	float transformedY = m_values[Iy]*vectorQuantityXYZ.x + m_values[Jy]*vectorQuantityXYZ.y + m_values[Ky]*vectorQuantityXYZ.z;
	float transformedZ = m_values[Iz]*vectorQuantityXYZ.x + m_values[Jz]*vectorQuantityXYZ.y + m_values[Kz]*vectorQuantityXYZ.z;

	return Vec3(transformedX, transformedY, transformedZ);
}

Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	float transformedX = m_values[Ix]*positionXY.x + m_values[Jx]*positionXY.y + m_values[Tx];
	float transformedY = m_values[Iy]*positionXY.x + m_values[Jy]*positionXY.y + m_values[Ty];

	return Vec2(transformedX, transformedY);
}

Vec3 const Mat44::TransformPosition3D(Vec3 const& positionXYZ) const
{
	float transformedX = m_values[Ix]*positionXYZ.x + m_values[Jx]*positionXYZ.y + m_values[Kx]*positionXYZ.z + m_values[Tx];
	float transformedY = m_values[Iy]*positionXYZ.x + m_values[Jy]*positionXYZ.y + m_values[Ky]*positionXYZ.z + m_values[Ty];
	float transformedZ = m_values[Iz]*positionXYZ.x + m_values[Jz]*positionXYZ.y + m_values[Kz]*positionXYZ.z + m_values[Tz];

	return Vec3(transformedX, transformedY, transformedZ);
}

Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	float transformedX = m_values[Ix]*homogeneousPoint3D.x + m_values[Jx]*homogeneousPoint3D.y + m_values[Kx]*homogeneousPoint3D.z + m_values[Tx]*homogeneousPoint3D.w;
	float transformedY = m_values[Iy]*homogeneousPoint3D.x + m_values[Jy]*homogeneousPoint3D.y + m_values[Ky]*homogeneousPoint3D.z + m_values[Ty]*homogeneousPoint3D.w;
	float transformedZ = m_values[Iz]*homogeneousPoint3D.x + m_values[Jz]*homogeneousPoint3D.y + m_values[Kz]*homogeneousPoint3D.z + m_values[Tz]*homogeneousPoint3D.w;
	float transformedW = m_values[Iw]*homogeneousPoint3D.x + m_values[Jw]*homogeneousPoint3D.y + m_values[Kw]*homogeneousPoint3D.z + m_values[Tw]*homogeneousPoint3D.w;

	return Vec4(transformedX, transformedY, transformedZ, transformedW);
}

float* Mat44::GetAsFloatArray()
{
	float* floatArray = new float[16];
	for (int i = 0; i < 16; i++)
	{
		floatArray[i] = m_values[i];
	}
	return floatArray;
}

float const* Mat44::GetAsFloatArray() const
{
	float const* floatArray = new float[16] {	
		m_values[0], m_values[1], m_values[2], m_values[3], m_values[4], m_values[5], m_values[6], m_values[7],
		m_values[8], m_values[9], m_values[10], m_values[11], m_values[12], m_values[13], m_values[14], m_values[15]
	};
	return floatArray;
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}


Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 translationInverse;
	translationInverse.m_values[Tx] = -m_values[Tx];
	translationInverse.m_values[Ty] = -m_values[Ty];
	translationInverse.m_values[Tz] = -m_values[Tz];

	Mat44 rotationInverse;
	rotationInverse.SetIJK3D(GetIBasis3D(), GetJBasis3D(), GetKBasis3D());
	rotationInverse.Transpose();

	Mat44 orthonormalInverse;

	orthonormalInverse.Append(rotationInverse);
	orthonormalInverse.Append(translationInverse);
	return orthonormalInverse;
}

float Mat44::GetUniformScale() const
{
	return fabsf(GetIBasis3D().GetLength());
}

Vec3 Mat44::GetScale() const
{
	return Vec3(GetIBasis3D().GetLength(), GetJBasis3D().GetLength(), GetKBasis3D().GetLength());
}

EulerAngles Mat44::GetEulerAngles()
{
	float yaw = Atan2Degrees(m_values[Jx], m_values[Ix]);
	float pitch = ConvertRadiansToDegrees(asinf(m_values[Kx]));
	float roll = Atan2Degrees(m_values[Ky], m_values[Kz]);
	return EulerAngles(yaw, pitch, roll);
}

Mat44 Mat44::GetNormalizedIJKMatrix() const
{
	Mat44 ijkBasisMatrix;
	ijkBasisMatrix.SetIJK3D(GetIBasis3D().GetNormalized(), GetJBasis3D().GetNormalized(), GetKBasis3D().GetNormalized());
	return ijkBasisMatrix;
}

void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;

}

void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1;
}

void Mat44::SetScale(Vec3 const& newScale)
{
	SetIJK3D(GetIBasis3D().GetNormalized() * newScale.x, GetJBasis3D().GetNormalized() * newScale.y, GetKBasis3D().GetNormalized() * newScale.z);
}


void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

}

void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0;
	m_values[Jw] = 0;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0;
	m_values[Tw] = 1;
}

void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0;
}

void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1;
}

void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

void Mat44::Transpose()
{
	Mat44 copyOfThis = *this;
	float const* beforeValues = copyOfThis.m_values;

	m_values[Jx] = beforeValues[Iy];
	m_values[Iy] = beforeValues[Jx];

	m_values[Kx] = beforeValues[Iz];
	m_values[Iz] = beforeValues[Kx];

	m_values[Tx] = beforeValues[Iw];
	m_values[Iw] = beforeValues[Tx];

	m_values[Ky] = beforeValues[Jz];
	m_values[Jz] = beforeValues[Ky];

	m_values[Ty] = beforeValues[Jw];
	m_values[Jw] = beforeValues[Ty];

	m_values[Tz] = beforeValues[Kw];
	m_values[Kw] = beforeValues[Tz];
}

void Mat44::Orthonormalize_IFwd_JLeft_KUp()
{
	Vec3 iBasis = GetIBasis3D();
	iBasis = iBasis.GetNormalized();

	Vec3 kBasis = GetKBasis3D();
	Vec3 badK = GetProjectedOnto3D(kBasis, iBasis);
	kBasis -= badK;
	kBasis = kBasis.GetNormalized();

	Vec3 jBasis = CrossProduct3D(kBasis, iBasis);
	SetIJK3D(iBasis, jBasis, kBasis);
}

void Mat44::Orthonormalize_IFwd_JLeft_KUp_PreserveK()
{
	Vec3 kBasis = GetKBasis3D();
	kBasis = kBasis.GetNormalized();

	Vec3 iBasis = GetIBasis3D();
	Vec3 badi = GetProjectedOnto3D(iBasis, kBasis);
	iBasis -= badi;
	iBasis = iBasis.GetNormalized();

	Vec3 jBasis = CrossProduct3D(kBasis, iBasis);
	SetIJK3D(iBasis, jBasis, kBasis);
}

void Mat44::Append(Mat44 const& appendThis)
{
	Mat44 copyOfThis = *this;
	float const* r = appendThis.m_values;
	float const* l = copyOfThis.m_values;

	m_values[Ix] = l[Ix]*r[Ix] + l[Jx]*r[Iy] + l[Kx]*r[Iz] + l[Tx]*r[Iw];
	m_values[Iy] = l[Iy]*r[Ix] + l[Jy]*r[Iy] + l[Ky]*r[Iz] + l[Ty]*r[Iw];
	m_values[Iz] = l[Iz]*r[Ix] + l[Jz]*r[Iy] + l[Kz]*r[Iz] + l[Tz]*r[Iw];
	m_values[Iw] = l[Iw]*r[Ix] + l[Jw]*r[Iy] + l[Kw]*r[Iz] + l[Tw]*r[Iw];

	m_values[Jx] = l[Ix]*r[Jx] + l[Jx]*r[Jy] + l[Kx]*r[Jz] + l[Tx]*r[Jw];
	m_values[Jy] = l[Iy]*r[Jx] + l[Jy]*r[Jy] + l[Ky]*r[Jz] + l[Ty]*r[Jw];
	m_values[Jz] = l[Iz]*r[Jx] + l[Jz]*r[Jy] + l[Kz]*r[Jz] + l[Tz]*r[Jw];
	m_values[Jw] = l[Iw]*r[Jx] + l[Jw]*r[Jy] + l[Kw]*r[Jz] + l[Tw]*r[Jw];
	
	m_values[Kx] = l[Ix]*r[Kx] + l[Jx]*r[Ky] + l[Kx]*r[Kz] + l[Tx]*r[Kw];
	m_values[Ky] = l[Iy]*r[Kx] + l[Jy]*r[Ky] + l[Ky]*r[Kz] + l[Ty]*r[Kw];
	m_values[Kz] = l[Iz]*r[Kx] + l[Jz]*r[Ky] + l[Kz]*r[Kz] + l[Tz]*r[Kw];
	m_values[Kw] = l[Iw]*r[Kx] + l[Jw]*r[Ky] + l[Kw]*r[Kz] + l[Tw]*r[Kw];

	m_values[Tx] = l[Ix]*r[Tx] + l[Jx]*r[Ty] + l[Kx]*r[Tz] + l[Tx]*r[Tw];
	m_values[Ty] = l[Iy]*r[Tx] + l[Jy]*r[Ty] + l[Ky]*r[Tz] + l[Ty]*r[Tw];
	m_values[Tz] = l[Iz]*r[Tx] + l[Jz]*r[Ty] + l[Kz]*r[Tz] + l[Tz]*r[Tw];
	m_values[Tw] = l[Iw]*r[Tx] + l[Jw]*r[Ty] + l[Kw]*r[Tz] + l[Tw]*r[Tw];
}

void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Mat44 zRotationMat = Mat44::CreateZRotationDegrees(degreesRotationAboutZ);
	Append(zRotationMat);
}

void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Mat44 yRotationMat = Mat44::CreateYRotationDegrees(degreesRotationAboutY);
	Append(yRotationMat);
}

void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Mat44 xRotationMat = CreateXRotationDegrees(degreesRotationAboutX);
	Append(xRotationMat);
}

void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Mat44 translationMat2D = Mat44::CreateTranslation2D(translationXY);;
	Append(translationMat2D);
}

void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 translationMat3D = Mat44::CreateTranslation3D(translationXYZ);
	Append(translationMat3D);
}

void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Mat44 uniformScaleMat2D = Mat44::CreateUniformScale2D(uniformScaleXY);
	Append(uniformScaleMat2D);
}

void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Mat44 uniformScaleMat3D = Mat44::CreateUniformScale3D(uniformScaleXYZ);
	Append(uniformScaleMat3D);
}

void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 nonUniformScaleMat2D = Mat44::CreateNonUniformScale2D(nonUniformScaleXY);
	Append(nonUniformScaleMat2D);
}

void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 nonUniformScaleMat3D = Mat44::CreateNonUniformScale3D(nonUniformScaleXYZ);
	Append(nonUniformScaleMat3D);
}


