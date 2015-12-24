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

#include "winmultithread.h"

#ifdef SORT_IN_WINDOWS
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
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

// critical section
struct CSection{
	CRITICAL_SECTION cs;

	CSection(){
		InitializeCriticalSection(&cs);
	}
	~CSection(){
		DeleteCriticalSection(&cs);
	}
}gCS;

static unsigned int WINAPI RenderThread_Win_Run(LPVOID lpParameter)
{
	// get win thread
	RenderThreadWin* renderThreadWin = (RenderThreadWin*)lpParameter;

	// setup lts
	g_ThreadId = renderThreadWin->GetThreadID();

	// run the thread
	renderThreadWin->RunThread();

	// end the thread
	renderThreadWin->EndThread();

	return 0;
}

RenderThreadWin::RenderThreadWin(unsigned tid) :m_tid(tid)
{
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
	// the thread is finished
	m_finished = true;
}

// Run the thread
void RenderThreadWin::RunThread()
{
	while (true)
	{
		EnterCriticalSection(&gCS.cs);
		if (RenderTaskQueue::GetSingleton().IsEmpty())
		{
			LeaveCriticalSection(&gCS.cs);
			break;
		}
		// Get a new task from the task queue
		RenderTask& task = RenderTaskQueue::GetSingleton().PopTask();
		LeaveCriticalSection(&gCS.cs);

		// execute the task
		task.Execute(m_pIntegrator);

		// Destroy the task
		RenderTask::DestoryRenderTask(task);
	}
}

MutexWin::MutexWin()
{
	InitializeCriticalSection(&m_cs);
}

MutexWin::~MutexWin()
{
	DeleteCriticalSection(&m_cs);
}

void MutexWin::Lock()
{
	EnterCriticalSection(&m_cs);
}

void MutexWin::Unlock()
{
	LeaveCriticalSection(&m_cs);
}

#endif