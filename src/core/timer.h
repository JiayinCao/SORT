/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.

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

#include <chrono>
#include "core/log.h"
#include "core/stats.h"

//! @brief  Timer is for evaluating time elapsed for a specific operation.
/**
 * Timer is a tiny class for evaluating elapsed time during an operation.
 * Since there is no global state involved, one can use multiple Timer at
 * the same time in multiple threads. The time window could even overlap
 * with each other.
 */
class   Timer
{
    using clock = std::chrono::high_resolution_clock;

public:
    //! @brief  Constructor will choose to start timer automatically.
    Timer():m_start(clock::now()){}

    //! @brief  Reset the timer.
    inline void Reset() {
        m_start = clock::now();
    }

    //! @brief  Get elapsed time since last time the timer is reset.
    //!
    //! This function won't reset the timer after returning the value.
    //! The time will continuing evaluating time.
    //!
    //! @return Get the elapsed time in million second since last
    //!         time the timer is reset.
    inline unsigned int GetElapsedTime() const {
        return (unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - m_start).count();
    }

private:
    std::chrono::time_point<clock>  m_start;        /**< Start point of last time timer is triggered. */
};

//! @brief  A simple wrapper for outputing elapsed time during the life time of its instance.
class   TimerWrapper{
public:
    //! @brief  Constructor.
    //!
    //! @param  name    Name of the timer.
    //! @param  stat    SORT Stats profiling counter.
    TimerWrapper( const std::string name , StatsInt* stat = nullptr ):m_name(name),m_stat(stat){}

    //! @brief  Report elapsed time during the life time of this instance.
    ~TimerWrapper(){
        if( !m_name.empty() )
            slog(INFO, GENERAL, "%s costs %f (s).", m_name.c_str() , (float)(m_timer.GetElapsedTime() / 1000.0f) );
        if( m_stat )
            *m_stat = m_timer.GetElapsedTime();
    }

private:
    Timer           m_timer;    /**< Timer of the wrapper. */
    std::string     m_name;     /**< Name of the wrapper. */
    StatsInt*       m_stat;     /**< Pointer to stat data. */
};

#define TIMING_EVENT( name )                TimerWrapper    localTimerWrapper(name);
#define TIMING_EVENT_STAT( name , stat )    TimerWrapper    localTimerWrapper(name,&stat);