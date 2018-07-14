/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include the header
#include "timer.h"
#include "utility/log.h"

#if defined(SORT_IN_LINUX) || defined(SORT_IN_MAC)
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#elif defined(SORT_IN_WINDOWS)
#include <windows.h>
#endif

#include <time.h>

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
        slog( WARNING , GENERAL , "Timer is already set." );
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
        slog( WARNING , GENERAL , "Timer is not set." );
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
        slog( WARNING , GENERAL , "Trying to get elapsed time when timing, return 0." );
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
