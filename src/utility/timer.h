/*
 * filename :	Timer
 *
 * Programmer :	Cao Jiayin
 */

#ifndef	SORT_TIMER
#define	SORT_TIMER

#include "singleton.h"

////////////////////////////////////////////////////////////////////////////
// definition of timer
class	Timer : public Singleton<Timer>
{
// public method
public:
	// destructor
	~Timer(){}

	// create timer
	static void CreateTimer();

	// set the timer
	void StartTimer();
	// stop timer
	void StopTimer();

	// get elapsed time
	unsigned long GetElapsedTime() const;
	// get total elapsed time
	unsigned long GetTotalElapsedTime() const;

	// reset the timer
	void ResetTimer();

// private field
private:
	// the time
	unsigned long m_elapsed;
	unsigned long m_totalElapsed;

	// whether timer is set
	bool	m_bTimerSet;

	// constructor
	Timer() { ResetTimer(); }
};

#endif
