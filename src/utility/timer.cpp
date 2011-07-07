/*
 * filename :	Timer.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header
#include "timer.h"
#include "managers/logmanager.h"

#if defined(SORT_IN_LINUX) 
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#elif defined(SORT_IN_WINDOWS)
#include <windows.h>
#endif

DEFINE_SINGLETON(Timer);

// get tick count
unsigned long getTickCount()
{
	unsigned long currentTime;

#if defined(SORT_IN_LINUX)
	struct timeval current;
	gettimeofday( &current , 0 );
	currentTime = current.tv_sec * 1000 + current.tv_usec/1000;
#elif defined(SORT_IN_WINDOWS)
	currentTime = GetTickCount();
#endif

	return currentTime;
}

// start timer
void Timer::StartTimer()
{
	if( m_bTimerSet )
	{
		LOG_WARNING<<"Timer is already set."<<ENDL;
		return;
	}
	// get curren tick count
	m_elapsed = getTickCount();

	m_bTimerSet = true;
}

// stop timer
void Timer::StopTimer()
{
	if( m_bTimerSet == false )
	{
		LOG_WARNING<<"Timer is not set."<<ENDL;
		return;
	}
	m_elapsed = getTickCount() - m_elapsed;

	m_totalElapsed += m_elapsed;

	m_bTimerSet = false;
}

// get elapsed time
unsigned long Timer::GetElapsedTime() const
{
	if( m_bTimerSet )
	{
		LOG_WARNING<<"Trying to get elapsed time when timing, return 0."<<ENDL;
		return 0L;
	}
	return m_elapsed;
}

// get total elapsed time
unsigned long Timer::GetTotalElapsedTime() const
{
	return m_totalElapsed;
}

// reset the timer
void Timer::ResetTimer()
{
	m_bTimerSet = false;
	m_elapsed = 0;
	m_totalElapsed = 0;
}
