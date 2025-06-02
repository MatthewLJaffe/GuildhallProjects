#pragma once

class Clock;

// Timer class that can be attached to any clock in a heirarchy and correctly handles duration
// regardless of frequency
class Timer
{
public:
	// Create a timer with a period and the specified clock. If the clock 
	// is null, use the system clock
	explicit Timer(float period = 1.f, const Clock* clock = nullptr);

	// Set the start time to the clock's current total time.
	void Start();

	// Sets the start time back to zero.
	void Stop();

	void SetTimePosition(float timePosition);

	// Returns zero if time stopped, otherwise returns the time elapsed between the clock's current
	// time and our start time.
	float GetElapsedTime() const;

	// Return the elapsed time as a percentage of our period. This can be greater than 1. 
	float GetElapsedFraction() const;

	// returns true if our start time is zero.
	bool IsStopped() const;

	//returns true if our elapsed time is greater than our period and we are not stopped
	bool HasPeriodElapsed() const;

	// If a period has elapsed and we are not stopped, decrements a period by adding a
	// period to the start time and returns true. Generally called within a loop until it
	// returns false so the caller can process each elapsed period.
	bool DecrementPeriodIfElapsed();

	// The clock to use for getting the current time.
	const Clock* m_clock = nullptr;

	// Clock time at which the timer has started. This is incremented by one period each
	// time we decrement a period, however, so it is not an absolute start time. It is
	// the start time of all periods that have not yet been decremented.
	float m_startTime = 0.0f;

	//The time interval it takes for a period to elapse
	float m_period = 0.f;
};