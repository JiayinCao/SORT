/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.

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

#if (defined(STBI__X86_TARGET) || defined(STBI__X64_TARGET))
#include <emmintrin.h>
#endif

#include <thread>
#include <atomic>
#include "core/define.h"

class WorkerThread{
public:
    // Constructor
    WorkerThread( unsigned tid ) : m_tid(tid) {}

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
};

class spinlock_mutex{
public:
    void lock() {
        // std::memory_order_acquire is neccessary here to prevent the out-of-order execution optimization.
        // It makes sure all memory load will happen after the lock is acquired.
        while (locked.test_and_set(std::memory_order_acquire)) { 
            // In a very contended multi-threading environment, full busy loop may not be the most efficient thing to do since
            // they consume CPU cycles all the time. This instruction could allow delaying CPU instructions for a few cycles in
            // some cases to allow other threads to take ownership of hardware resources.
            // https://software.intel.com/en-us/comment/1134767
            #if (defined(STBI__X86_TARGET) || defined(STBI__X64_TARGET))
            _mm_pause(); 
            #endif
        }
    }
    void unlock() {
        // std::memory_order_release will make sure all memory writting operations will be finished by the time this is done.
        locked.clear(std::memory_order_release);
    }

private:
    std::atomic_flag locked = ATOMIC_FLAG_INIT ;
};
