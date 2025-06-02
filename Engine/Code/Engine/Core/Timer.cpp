#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"

Timer::Timer(float period, Clock const* clock)
	: m_period(period)
{
	if (clock == nullptr)
	{
		m_clock = &Clock::GetSystemClock();
	}
	else
	{
		m_clock = clock;
	}
}

void Timer::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
	if (m_startTime == 0.f)
	{
		m_startTime = .0001f;
	}
}

void Timer::Stop()
{
	m_startTime = 0;
}

void Timer::SetTimePosition(float timePosition)
{
	m_startTime = m_clock->GetTotalSeconds() - timePosition;
}

float Timer::GetElapsedTime() const
{
	if (m_clock->IsPaused())
	{
		//return 0;
	}

	return m_clock->GetTotalSeconds() - m_startTime;
}

float Timer::GetElapsedFraction() const
{
	return Clamp(GetElapsedTime() / m_period, 0.f, 1.f);
}

bool Timer::IsStopped() const
{
	return m_startTime == 0;
}

bool Timer::HasPeriodElapsed() const
{
	return GetElapsedTime() > m_period && !IsStopped();
}

bool Timer::DecrementPeriodIfElapsed()
{
	if (HasPeriodElapsed() && !IsStopped())
	{
		m_startTime += m_period;
		return true;
	}
	return false;
}


