#pragma once
struct Vec2;
struct Vec3;
struct Vec4;
struct EulerAngles;

struct Mat44
{
	enum { Ix,Iy,Iz,Iw, Jx,Jy,Jz,Jw, Kx,Ky,Kz,Kw, Tx,Ty,Tz,Tw }; // index nicknames 0 through 15
	float m_values[16];

	Mat44(); // Default constructor is IDENTITY matrix NOT all zeros

	bool		operator==(const Mat44& compare) const;		// vec2 == vec2
	explicit Mat44( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D );
	explicit Mat44( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D );
	explicit Mat44( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D );
	explicit Mat44(const float values[16]);


	static Mat44 const CreateTranslation2D( Vec2 const& translationXY );
	static Mat44 const CreateTranslation3D( Vec3 const& translationXYZ );
	static Mat44 const CreateUniformScale2D( float uniformScaleXY );
	static Mat44 const CreateUniformScale3D( float uniformScaleXYZ );
	static Mat44 const CreateNonUniformScale2D( Vec2 const& nonUniformScaleXY );
	static Mat44 const CreateNonUniformScale3D( Vec3 const& nonUniformScaleXYZ );
	static Mat44 const CreateZRotationDegrees( float rotationDegreesAboutZ );
	static Mat44 const CreateYRotationDegrees( float rotationDegreesAboutY );
	static Mat44 const CreateXRotationDegrees( float rotationDegreesAboutX );
	static Mat44 const CreateOrthoProjection( float left, float right, float bottom, float top, float zNear, float zFar);
	static Mat44 const CreatePerspectiveProjection( float fovYDegrees, float aspect, float zNear, float zFar );
	static Mat44 const CreateInversePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar);
	static Mat44 const CreateAxisAngleRotation(Vec3 const& axis, float angle);

	Vec2 const TransformVectorQuantity2D( Vec2 const& vectorQuantityXY ) const;
	Vec3 const TransformVectorQuantity3D( Vec3 const& vectorQuantityXYZ ) const;
	Vec2 const TransformPosition2D( Vec2 const& positionXY ) const;
	Vec3 const TransformPosition3D( Vec3 const& positionXYZ ) const;
	Vec4 const TransformHomogeneous3D( Vec4 const& homogeneousPoint3D ) const;

	float* GetAsFloatArray();
	float const* GetAsFloatArray() const;
	Vec2 const GetIBasis2D() const;
	Vec2 const GetJBasis2D() const;
	Vec2 const GetTranslation2D() const;
	Vec3 const GetIBasis3D() const;
	Vec3 const GetJBasis3D() const;
	Vec3 const GetKBasis3D() const;
	Vec3 const GetTranslation3D() const;
	Vec4 const GetIBasis4D() const;
	Vec4 const GetJBasis4D() const;
	Vec4 const GetKBasis4D() const;
	Vec4 const GetTranslation4D() const;
	Mat44 const GetOrthonormalInverse() const;
	float GetUniformScale() const;
	Vec3 GetScale() const;

	EulerAngles GetEulerAngles();
	Mat44 GetNormalizedIJKMatrix() const;

	void SetTranslation2D( Vec2 const& translationXY );		//Sets translationZ = 0, translationW = 1
	void SetTranslation3D( Vec3 const& translationXYZ );	//Sets translationW = 1
	void SetScale(Vec3 const& newScale);
	void SetIJ2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D );	//Sets z=0, w=0, for i & j; does not modify k or t
	void SetIJT2D( Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY ); // Sets z=0, w=0 for i,j,t does not modify k
	void SetIJK3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D );		// Sets w=0 for i,j,k
	void SetIJKT3D( Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ ); // Sets w=0 for i,j,k, w=1 for t
	void SetIJKT4D( Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D );
	void Transpose();
	void Orthonormalize_IFwd_JLeft_KUp();
	void Orthonormalize_IFwd_JLeft_KUp_PreserveK();


	void Append( Mat44 const& appendThis );
	void AppendZRotation( float degreesRotationAboutZ );
	void AppendYRotation( float degreesRotationAboutY );
	void AppendXRotation( float degreesRotationAboutX );
	void AppendTranslation2D( Vec2 const& translationXY );
	void AppendTranslation3D( Vec3 const& translationXYZ );
	void AppendScaleUniform2D(float uniformScaleXY);
	void AppendScaleUniform3D(float uniformScaleXYZ);
	void AppendScaleNonUniform2D( Vec2 const& nonUniformScaleXY );
	void AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ);
};