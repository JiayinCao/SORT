/*
	FileName:      stdthread.h

	Created Time:  2016-05-31

	Auther:        Cao Jiayin

	Email:         soraytrace@hotmail.com

	Location:      China, Shanghai

	Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
				'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
				modify or publish the source code. It's cross platform. You could compile the source code in
				linux and windows , g++ or visual studio 2008 is required.
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