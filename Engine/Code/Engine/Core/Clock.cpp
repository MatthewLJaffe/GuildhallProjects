#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"

static Clock s_systemClock = Clock();

Clock::Clock()
{
	if (this == &s_systemClock)
	{
		m_parent = nullptr;
	}
	else
	{
		m_parent = &s_systemClock;
		m_parent->AddChild(this);
	}
}

Clock::Clock(Clock& parent)
	: m_parent(&parent)
{
	m_parent->AddChild(this);
}

Clock::~Clock()
{
	if (m_parent != nullptr)
	{
		for (size_t i = 0; i < m_parent->m_children.size(); i++)
		{
			if (m_parent->m_children[i] == this)
			{
				m_parent->m_children.erase(m_parent->m_children.begin() + i);
				break;
			}
		}
	}
	m_parent = nullptr;
	for (size_t i = 0; i < m_children.size(); i++)
	{
		m_children[i]->m_parent = nullptr;
	}
}

void Clock::Reset()
{
	// set the last updated time to be the current system time
	m_lastUpdateTimeInSeconds = GetSystemClock().GetTotalSeconds();
	m_totalSeconds = 0.0f;
	m_deltaSeconds = 0.0f;
	m_frameCount = 0;
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::Unpause()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::SetSingleFrame()
{
	m_stepSingleFrame = true;
	m_isPaused = false;
}

void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}

Clock& Clock::GetSystemClock()
{
	return s_systemClock;
}

void Clock::TickSystemClock()
{
	s_systemClock.Tick();
}

void Clock::Tick()
{
	double currentTime = GetCurrentTimeSeconds();
	m_deltaSeconds = static_cast<float>(currentTime - m_lastUpdateTimeInSeconds);
	m_deltaSeconds = Clamp(m_deltaSeconds, 0.f, .1f);
	m_lastUpdateTimeInSeconds = static_cast<float>(currentTime);
	m_frameCount++;
	Advance(m_deltaSeconds);
}

void Clock::Advance(float deltaTimeSeconds)
{
	if (m_isPaused)
	{
		deltaTimeSeconds *= 0.f;
	}
	if (m_stepSingleFrame)
	{
		m_isPaused = true;
		m_stepSingleFrame = false;
	}
	deltaTimeSeconds *= m_timeScale;
	m_totalSeconds += deltaTimeSeconds;
	for (size_t i = 0; i < m_children.size(); i++)
	{
		m_children[i]->Advance(deltaTimeSeconds);
	}
	m_deltaSeconds = deltaTimeSeconds;
}

void Clock::AddChild(Clock* childClock)
{
	m_children.push_back(childClock);
	childClock->m_parent = this;
}

void Clock::RemoveChild(Clock* childClock)
{
	for (size_t i = 0; i < m_children.size(); i++)
	{
		if (childClock == m_children[i])
		{
			m_children[i]->m_parent = nullptr;
			m_children.erase(m_children.begin() + i);
			break;
		}
	}
}


