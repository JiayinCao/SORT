/*
   FileName:      timer.cpp

   Created Time:  2011-08-04 12:43:49

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "timer.h"
#include "managers/logmanager.h"

#if defined(SORT_IN_LINUX) || defined(SORT_IN_MAC)
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#elif defined(SORT_IN_WINDOWS)
#include <windows.h>
#endif

#include <time.h>

DEFINE_SINGLETON(Timer);

// get tick count
unsigned long getTickCount()
{
	unsigned long currentTime;

#if defined(SORT_IN_LINUX) || defined(SORT_IN_MAC)
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
unsigned Timer::StopTimer()
{
	if( m_bTimerSet == false )
	{
		LOG_WARNING<<"Timer is not set."<<ENDL;
		return 0;
	}
	m_elapsed = getTickCount() - m_elapsed;

	m_totalElapsed += m_elapsed;

	m_bTimerSet = false;

	return m_elapsed;
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

// Get Current Date
string Timer::GetDate()
{
	// Get Current date
	time_t tt = time(NULL);
	tm* t= localtime(&tt);
	char time_str[1024];
	sprintf(time_str, "%d_%02d_%02d-%02d_%02d_%02d.bmp", 
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec);
	return string(time_str);
}
