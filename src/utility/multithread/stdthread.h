/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "sort.h"

#ifndef SORT_STDTHREAD
#define SORT_STDTHREAD

#include <thread>
#include <mutex>

class Integrator;

// get the thread id
int ThreadId();

class RenderThreadStd
{
	// public method
public:
	// constructor
	RenderThreadStd(unsigned tid);

	// Begin thread
	void BeginThread();

	// End thread
	void EndThread();

	// Run the thread
	void RunThread();

	// Whether the thread is finished
	bool IsFinished() {
		return m_finished;
	}

	// get thread id
	int GetThreadID() const {
		return m_tid;
	}

// private field
private:
	// the thread id
	unsigned m_tid;
	// whether the thread is finished
	bool	m_finished;

// the rendering data
public:
	Integrator*	m_pIntegrator = nullptr;
};

class MutexStd
{
public:
	// lock/unlock
	void Lock();
	void Unlock();

private:
	// critical section
	std::mutex	m_mutex;
};

#define PlatformThreadUnit	RenderThreadStd
#define PlatformMutex		MutexStd

#endif
