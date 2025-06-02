#include <vector>

class Clock
{
public:
	// Default constructor, uses the system clock as the parent of the new clock
	Clock();

	// Constructor to specify a parent clock for the new clock
	explicit Clock(Clock& parent);

	// Destructor, unparents ourself and our children to avoid crashes but does not otherwise
	// Try to fix up the clock heirarchy. That is the responsibility of the user of this class.
	~Clock();
	Clock(const Clock& copy) = delete;

	// Reset all book keeping variable values back to zero and then set the last updated time
	// to be the current system time
	void Reset();

	bool IsPaused() const;
	void Pause();
	void Unpause();
	void TogglePause();

	// Unpause for frame then pause again the next frame
	void SetSingleFrame();

	//Set and get the value by which this clock scales delta seconds
	void SetTimeScale(float timeScale);
	float GetTimeScale() const;

	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;
	size_t GetFrameCount() const;

public:
	// Returns a refrence to a static system clock that by default will be the parent of all
	// other clocks if a parent is not specified
	static Clock& GetSystemClock();

	// Called in BeginFrame to Tick the system clock, which will in turn Advance the system
	// clock, which will in return Advance all of its children, thus updating the entire heirarchy.
	static void TickSystemClock();

protected:
	// Calculates the current delta seconds and clamps it to the max delta time, sets the last
	//updated time, then calls Advance, passing down the delta seconds.
	void Tick();

	// Calculates delta seconds based on pausing and time scale, updates all remaining book 
	// keeping variables, calls Advance on all child clocks and passes down our delta seconds,
	// and handles pausing after frames for stepping single frames
	void Advance(float deltaTimeSeconds);

	// Add a child clock as one of our children. Does not handle cases where the child clock
	// already has a parent.
	void AddChild(Clock* childClock);

	// Removes a child clock from our children if it is a child, otherwise does nothing.
	void RemoveChild(Clock* childClock);

protected:
	// Parent clock. Will be nullptr for the root clock
	Clock* m_parent = nullptr;

	// All children of this clock.
	std::vector<Clock*> m_children;

	// Book keeping variables.
	float m_lastUpdateTimeInSeconds = 0.0f;
	float m_totalSeconds = 0.0f;
	float m_deltaSeconds = 0.0f;
	size_t m_frameCount = 0;

	// Time scale for this clock.
	float m_timeScale = 1.0f;

	// Pauses the clock completely.
	bool m_isPaused = false;

	// For single stepping frames.
	bool m_stepSingleFrame = false;

	// Max delta time. Useful for preventing large time steps when stepping in a debugger.
	float m_maxDeltaSeconds = 0.1f;
};