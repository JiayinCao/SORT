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

#include "thread.h"

#include "managers/memmanager.h"
#include "core/stats.h"
#include "core/profile.h"
#include "task/task.h"
#include "core/profile.h"
#include "core/define.h"

// thread id
static Thread_Local int g_ThreadId = 0;

// get the thread id
int ThreadId()
{
	return g_ThreadId;
}

// get the number of cpu cores in the system
unsigned NumSystemCores()
{
	return std::thread::hardware_concurrency();
}

// critical section
spinlock_mutex g_mutex;

void WorkerThread::BeginThread()
{
	m_thread = std::thread([&]() {
		// setup lts
        g_ThreadId = m_tid;

		// run the thread
		RunThread();
	});
}

// Run the thread
void WorkerThread::RunThread()
{
    SORT_PROFILE("Rendering Thread")

	EXECUTING_TASKS();

    SortStatsFlushData();
}
