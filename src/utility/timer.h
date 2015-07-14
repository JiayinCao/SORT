/*
   FileName:      timer.h

   Created Time:  2011-08-04 12:43:54

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
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

	// set the timer
	void StartTimer();
	// stop timer
	unsigned StopTimer();

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

	friend class Singleton<Timer>;
};

#endif
