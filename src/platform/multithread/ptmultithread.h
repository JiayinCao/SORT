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

#ifndef SORT_PTMULTITHREAD
#define SORT_PTMULTITHREAD

#include "utility/define.h"

#if defined(SORT_IN_MAC) || defined(SORT_IN_LINUX)

#include <pthread.h>

// get the thread id
int ThreadId();

class Integrator;

class RenderThreadMac
{
	// public method
public:
	// constructor
	RenderThreadMac(unsigned tid);
	RenderThreadMac();

	// Begin thread
    void BeginThread();

	// End thread
	void EndThread();

	// Run the thread
    void RunThread();

	// Whether the thread is finished
	bool IsFinished(){
		return m_finished;
	}

    // get thread id
    int GetThreadID() const{
        return m_tid;
    }
    
	// private field
private:
	// thread handle
	pthread_t m_thread;
	// the thread id
	unsigned m_tid;
	// whether the thread is finished
	bool	m_finished;
	
// the rendering data
public:
	Integrator*	m_pIntegrator;
};

class MutexMac
{
public:
    MutexMac();
    ~MutexMac();
    
    void Lock();
    void Unlock();
    
private:
    pthread_mutex_t m_mutex;
};

#define PlatformThreadUnit	RenderThreadMac
#define PlatformMutex       MutexMac

#endif

#endif
