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
#include <process.h>
#include "taskqueue.h"
#include "managers\memmanager.h"
#include "system.h"

#ifdef SORT_IN_WINDOWS

extern System g_System;

__declspec(thread) int RenderThreadWin::m_WinThreadId = 0;

static unsigned int WINAPI RenderThread_Win_Run(LPVOID lpParameter)
{
	RenderThreadWin* renderThreadWin = (RenderThreadWin*) lpParameter;

	// run the thread
	renderThreadWin->RunThread();

	return 0;
}

// critical section
CRITICAL_SECTION gCS;

RenderThreadWin::RenderThreadWin(unsigned tid):m_tid(tid)
{
	m_WinThreadId = tid;
}

RenderThreadWin::~RenderThreadWin()
{
}

void RenderThreadWin::BeginThread()
{
	// Create end event
	m_endEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

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

		const RenderTask& task = RenderTaskQueue::GetSingleton().PopTask();
		
		LeaveCriticalSection(&gCS);

		unsigned right = task.ori_x + task.width;
		unsigned bottom = task.ori_y + task.height;

		g_System.RenderTile( task.ori_x , right , task.ori_y , bottom , m_tid );
	}
	
	// signal end event
	SetEvent(m_endEvent);
}

// wait thread for finish
void RenderThreadWin::WaitForFinish()
{
	WaitForSingleObject(m_endEvent,INFINITE);
}

#endif