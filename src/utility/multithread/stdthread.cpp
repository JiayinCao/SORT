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

#include "stdthread.h"

#include "managers/memmanager.h"

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
MutexStd g_mutex;

RenderThreadStd::RenderThreadStd(unsigned tid) :m_tid(tid)
{
	m_finished = false;
}

void RenderThreadStd::BeginThread()
{
	new std::thread([this]() {
		// setup lts
		g_ThreadId = GetThreadID();

		// run the thread
		RunThread();

		// end the thread
		EndThread();
	});
}

void RenderThreadStd::EndThread()
{
	// the thread is finished
	m_finished = true;
}

// Run the thread
void RenderThreadStd::RunThread()
{
	while (true)
	{
		g_mutex.Lock();
		if (RenderTaskQueue::GetSingleton().IsEmpty())
		{
			g_mutex.Unlock();
			break;
		}
		// Get a new task from the task queue
		RenderTask task = RenderTaskQueue::GetSingleton().PopTask();
		g_mutex.Unlock();

		// execute the task
		task.Execute(m_pIntegrator);

		// Destroy the task
		RenderTask::DestoryRenderTask(task);
	}
}

void MutexStd::Lock()
{
	m_mutex.lock();
}

void MutexStd::Unlock()
{
	m_mutex.unlock();
}
