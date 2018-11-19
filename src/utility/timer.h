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

#pragma once

#include "singleton.h"

////////////////////////////////////////////////////////////////////////////
// definition of timer
class	Timer : public Singleton<Timer>
{
public:
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
