/*
   FileName:      renderthread.h

   Created Time:  2015-07-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_RENDERTHREAD_WIN
#define SORT_RENDERTHREAD_WIN

#include "threadunit.h"

#ifdef SORT_IN_WINDOWS

#include <Windows.h>

class RenderThreadWin : public ThreadUnit
{
// public method
public:
	// constructor
	RenderThreadWin(unsigned tid);
	~RenderThreadWin();

	// Begin thread
	virtual void BeginThread();

	// End thread
	virtual void EndThread();

	// Run the thread
	virtual void RunThread();

	// Whether the thread is finished
	virtual bool IsFinished(){
		return m_finished;
	}

// private field
private:
	// the thread handle
	HANDLE	m_threadHandle;
	// the thread id
	unsigned m_tid;
	// whether the thread is finished
	bool	m_finished;

public:
	static __declspec(thread) int m_WinThreadId;
};

#endif

#endif