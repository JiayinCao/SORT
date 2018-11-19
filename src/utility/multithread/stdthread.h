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

#include "sort.h"

#pragma once

#include <thread>
#include <atomic>

class Integrator;

// get the thread id
int ThreadId();

class RenderThreadStd
{
public:
    // Constructor
    RenderThreadStd( unsigned tid , std::shared_ptr<Integrator> integrator ) : m_tid(tid) , m_pIntegrator( integrator ) {}
    
	// Begin thread
	void BeginThread();
    
	// Run the thread
	void RunThread();

	// Whether the thread is finished
	void Join() {
		return m_thread.join();
	}
    
private:
    std::thread m_thread;
    unsigned    m_tid = 0;
    
// the rendering data
public:
    std::shared_ptr<Integrator>	m_pIntegrator;
};

class spinlock_mutex
{
public:
    void lock() {
        while (locked.test_and_set(std::memory_order_acquire)) { ; }
    }
    void unlock() {
        locked.clear(std::memory_order_release);
    }

private:
    std::atomic_flag locked = ATOMIC_FLAG_INIT ;
};

#define PlatformThreadUnit      RenderThreadStd
#define PlatformMutex           std::mutex
#define PlatformSpinlockMutex   spinlock_mutex
