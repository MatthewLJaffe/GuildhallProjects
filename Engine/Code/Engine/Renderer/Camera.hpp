#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera
{
public:
	enum Mode
	{
		eMode_Orthographic,
		eMode_Perspective,
		eMode_Count
	};

	void SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.0f, float far = 1.0f);
	void SetPerspectiveView(float aspect, float fov, float near, float far);
	void SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	Mat44 GetRenderMatrix() const;
	void SetTransform(const Vec3& position, const EulerAngles& orientation);
	void SetFOV(float newFov);
	Mat44 GetViewMatrix() const;
	void SetTransform(Mat44 const& transform);
	Vec3 GetPosition();
	Vec3 GetCameraForward();
	Mat44 GetWorldTransform() const;
	Vec3 m_position;
	EulerAngles m_orientation;
	Mode m_mode = eMode_Orthographic;

	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	Vec2 GetOrthoDimensions() const;
	void Translate2D(Vec2 const& translation);

	Mat44 GetOrthographicMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetInversePerspectiveMatrix();
	Mat44 GetProjectionMatrix() const;
	AABB2 GetDirectXViewort() const;
	AABB2 m_normalizedViewport = AABB2::ZERO_TO_ONE;
	float GetAspectRatio() const;
	void SetAspectRatio(float newAspect);

	//functions that are depreciated but used in libra
	void SetCameraHeight(float newHeight);
	void SetCameraPos(Vec2 newCameraPos);
	Vec2 GetCameraPos();
	float GetPerspectiveNear() const;
	float GetPerspectiveFar() const;
	float GetPerspectiveFOV() const;
	Mat44 GetInverseViewProjectionMatrix();
	bool m_useWorldTransform = false;
private:
	Vec2 m_orthographicBottomLeft;
	Vec2 m_orthographicTopRight;
	float m_orthographicNear = 0.0f;
	float m_orthographicFar = 0.0f;

	float m_perspectiveAspect = 0.0f;
	float m_perspectiveFOV = 0.0f;
	float m_perspectiveNear = 0.0f;
	float m_perspectiveFar = 0.0f;

	Vec3 m_renderIBasis = Vec3(1.f, 0.0f, 0.0f);
	Vec3 m_renderJBasis = Vec3(0.f, 1.0f, 0.0f);
	Vec3 m_renderKBasis = Vec3(0.f, 0.0f, 1.0f);
	Mat44 m_worldTransform;
};