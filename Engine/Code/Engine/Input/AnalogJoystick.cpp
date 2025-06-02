#include "AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"

Vec2 AnalogJoystick::GetPosition() const
{
	return m_correctedPosition;
}

float AnalogJoystick::GetMagnitude() const
{
	return m_deadZoneCorrectedMag;
}

float AnalogJoystick::GetOrientationDegrees() const
{
	return m_deadZoneCorrectedAngleDegrees;
}

Vec2 AnalogJoystick::GetRawUncorrectedPosition() const
{
	return m_rawPosition;
}

float AnalogJoystick::GetInnerDeadZoneFraction() const
{
	return m_innerDeadZoneFraction;
}

float AnalogJoystick::GetOuterDeadZoneFraction() const
{
	return m_outerDeadZoneFraction;
}

void AnalogJoystick::Reset()
{
	m_correctedPosition = Vec2(0.f, 0.f);
	m_deadZoneCorrectedAngleDegrees = 0.f;
	m_deadZoneCorrectedMag = 0.f;
	m_rawPosition = Vec2(0.f, 0.f);
}

void AnalogJoystick::SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOutterDeadzoneThreshold)
{
	m_innerDeadZoneFraction = normalizedInnerDeadzoneThreshold;
	m_outerDeadZoneFraction = normalizedOutterDeadzoneThreshold;
}

void AnalogJoystick::UpdatePosition(float rawNormalizedX, float rawNormalizedY)
{
	m_rawPosition.x = rawNormalizedX;
	m_rawPosition.y = rawNormalizedY;
	m_deadZoneCorrectedAngleDegrees = m_rawPosition.GetOrientationDegrees();
	m_deadZoneCorrectedMag = RangeMapClamped(m_rawPosition.GetLength(), m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f);
	m_correctedPosition = Vec2::MakeFromPolarDegrees(m_deadZoneCorrectedAngleDegrees, m_deadZoneCorrectedMag);
}
