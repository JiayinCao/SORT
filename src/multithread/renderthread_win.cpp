/*
   FileName:      threadunit.cpp

   Created Time:  2015-07-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "renderthread_win.h"

#ifdef SORT_IN_WINDOWS
#include <process.h>
#include "taskqueue.h"
#include "managers/memmanager.h"
#include "system.h"

extern System g_System;

__declspec(thread) int RenderThreadWin::m_WinThreadId = 0;

static unsigned int WINAPI RenderThread_Win_Run(LPVOID lpParameter)
{
	RenderThreadWin* renderThreadWin = (RenderThreadWin*) lpParameter;

	// run the thread
	renderThreadWin->RunThread();

	// end the thread
	renderThreadWin->EndThread();

	return 0;
}

// critical section
CRITICAL_SECTION gCS;

RenderThreadWin::RenderThreadWin(unsigned tid):m_tid(tid)
{
	m_WinThreadId = tid;
	m_finished = false;
}

RenderThreadWin::~RenderThreadWin()
{
}

void RenderThreadWin::BeginThread()
{
	// Create the thread
	m_threadHandle = (HANDLE)_beginthreadex(
			nullptr,
			0,
			RenderThread_Win_Run,
			(void*)this,
			0,
			nullptr);
}

void RenderThreadWin::EndThread()
{
	// delete the integrator
	SAFE_DELETE(m_pIntegrator);

	// the thread is finished
	m_finished = true;
}

// Run the thread
void RenderThreadWin::RunThread()
{
	// setup thread id first
	m_WinThreadId = m_tid;

	while( true )
	{
		EnterCriticalSection(&gCS);
		if( RenderTaskQueue::GetSingleton().IsEmpty() )
		{
			LeaveCriticalSection(&gCS);
			break;
		}
		// Get a new task from the task queue
		RenderTask& task = RenderTaskQueue::GetSingleton().PopTask();
		LeaveCriticalSection(&gCS);

		// execute the task
		task.Execute(m_pIntegrator);

		// Destroy the task
		RenderTask::DestoryRenderTask( task );
	}
}

#endif