/*
	FileName:      stdthread.cpp

	Created Time:  2016-05-31

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
*/

#include "stdthread.h"

#include <process.h>
#include "managers/memmanager.h"

// thread id
static __declspec(thread) int g_ThreadId = 0;

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
		RenderTask& task = RenderTaskQueue::GetSingleton().PopTask();
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
