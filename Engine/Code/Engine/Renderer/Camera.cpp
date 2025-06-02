#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Window.hpp"

void Camera::SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near, float far)
{
	m_orthographicBottomLeft = bottomLeft;
	m_orthographicTopRight = topRight;
	m_orthographicNear = near;
	m_orthographicFar = far;
}

void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
 	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = near;
	m_perspectiveFar = far;
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_orthographicBottomLeft;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_orthographicTopRight;
}

Vec2 Camera::GetOrthoDimensions() const
{
	return Vec2(m_orthographicTopRight.x - m_orthographicBottomLeft.x, m_orthographicTopRight.y - m_orthographicBottomLeft.y);
}

void Camera::Translate2D(Vec2 const& translation)
{
	m_orthographicBottomLeft += translation;
	m_orthographicTopRight += translation;
}

Mat44 Camera::GetOrthographicMatrix() const
{
	return Mat44::CreateOrthoProjection(m_orthographicBottomLeft.x, m_orthographicTopRight.x, 
		m_orthographicBottomLeft.y, m_orthographicTopRight.y, m_orthographicNear, m_orthographicFar);
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
}

Mat44 Camera::GetInversePerspectiveMatrix()
{
	return Mat44::CreateInversePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
}

Mat44 Camera::GetProjectionMatrix() const
{
	Mat44 projectionMatrix;
	if (m_mode == eMode_Orthographic)
	{
		projectionMatrix = GetOrthographicMatrix();
	}
	else
	{
		projectionMatrix = GetPerspectiveMatrix();
	}
	projectionMatrix.Append(GetRenderMatrix());
	return projectionMatrix;
}

AABB2 Camera::GetDirectXViewort() const
{
	AABB2 dirextXViewport;

	dirextXViewport.m_mins.x = m_normalizedViewport.m_mins.x;
	dirextXViewport.m_mins.y = 1.f - m_normalizedViewport.m_mins.y;
	dirextXViewport.m_maxs.x = m_normalizedViewport.m_maxs.x;
	dirextXViewport.m_maxs.y = 1.f - m_normalizedViewport.m_maxs.y;

	float maxY = dirextXViewport.m_mins.y;
	dirextXViewport.m_mins.y = dirextXViewport.m_maxs.y;
	dirextXViewport.m_maxs.y = maxY;

	Vec2 screenDimensions;
	screenDimensions.x = (float)Window::GetTheWindowInstance()->GetClientDimensions().x;
	screenDimensions.y = (float)Window::GetTheWindowInstance()->GetClientDimensions().y;
	
	dirextXViewport.m_mins.x *= screenDimensions.x;
	dirextXViewport.m_maxs.x *= screenDimensions.x;
	dirextXViewport.m_mins.y *= screenDimensions.y;
	dirextXViewport.m_maxs.y *= screenDimensions.y;

	return dirextXViewport;
}

float Camera::GetAspectRatio() const
{
	Vec2 viewportDimensions = m_normalizedViewport.GetDimensions();
	float defaultAspectRatio = (float)Window::GetTheWindowInstance()->GetClientDimensions().x / (float)Window::GetTheWindowInstance()->GetClientDimensions().y;
	return defaultAspectRatio * (viewportDimensions.x / viewportDimensions.y);
}

void Camera::SetAspectRatio(float newAspect)
{
	m_perspectiveAspect = newAspect;
}

void Camera::SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

Mat44 Camera::GetRenderMatrix() const
{
	Mat44 renderBasis;
	renderBasis.SetIJK3D(m_renderIBasis, m_renderJBasis, m_renderKBasis);
	return renderBasis;
}

void Camera::SetTransform(const Vec3& position, const EulerAngles& orientation)
{
	m_position = position;
	m_orientation = orientation;

	Mat44 transform;
	transform.AppendTranslation3D(m_position);
	transform.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());

	m_worldTransform = transform;
}

void Camera::SetFOV(float newFov)
{
	m_perspectiveFOV = newFov;
}

Mat44 Camera::GetViewMatrix() const
{
	if (m_useWorldTransform)
	{
		return m_worldTransform.GetOrthonormalInverse();
	}
	Mat44 viewMatrix;
	Mat44 orientationMatrix = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	viewMatrix.AppendTranslation3D(m_position);
	viewMatrix.Append(orientationMatrix);
	return viewMatrix.GetOrthonormalInverse();
}

void Camera::SetTransform(Mat44 const& transform)
{
	m_worldTransform = transform;
}

Vec3 Camera::GetPosition()
{
	if (m_useWorldTransform)
	{
		return m_worldTransform.GetTranslation3D();
	}
	else
	{
		return m_position;
	}
}

Vec3 Camera::GetCameraForward()
{
	if (m_useWorldTransform)
	{
		return m_worldTransform.GetIBasis3D();
	}
	
	return m_orientation.GetIFwd();
}

Mat44 Camera::GetWorldTransform() const
{
	if (m_useWorldTransform)
	{
		return m_worldTransform;
	}

	Mat44 cameraWorldTransform;
	cameraWorldTransform.AppendTranslation3D(m_position);
	cameraWorldTransform.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
	return cameraWorldTransform;
}

void Camera::SetCameraHeight(float newHeight)
{
	AABB2 cameraBounds(m_orthographicBottomLeft, m_orthographicTopRight);
	float cameraAspect = cameraBounds.GetDimensions().x / cameraBounds.GetDimensions().y;

	m_orthographicBottomLeft = Vec2::ZERO;
	m_orthographicTopRight = Vec2(newHeight * cameraAspect, newHeight);
}

void Camera::SetCameraPos(Vec2 newCameraPos)
{
	AABB2 cameraBounds(m_orthographicBottomLeft, m_orthographicTopRight);
	cameraBounds.SetCenter(newCameraPos);
	m_orthographicBottomLeft = cameraBounds.m_mins;
	m_orthographicTopRight = cameraBounds.m_maxs;
}

Vec2 Camera::GetCameraPos()
{
	AABB2 cameraBounds(m_orthographicBottomLeft, m_orthographicTopRight);
	return cameraBounds.GetCenter();
}

float Camera::GetPerspectiveNear() const
{
	return m_perspectiveNear;
}

float Camera::GetPerspectiveFar() const
{
	return m_perspectiveFar;
}

float Camera::GetPerspectiveFOV() const
{
	return m_perspectiveFOV;
}

Mat44 Camera::GetInverseViewProjectionMatrix()
{
	Mat44 inverseView = GetViewMatrix().GetOrthonormalInverse();
	Mat44 inversePerspectiveMatrix = Mat44::CreateInversePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
	Mat44 inverseViewProjection;
	inverseViewProjection.Append(inverseView);
	inverseViewProjection.Append(inversePerspectiveMatrix);
	return inverseViewProjection;
}
