/*
   FileName:      threadunit.h

   Created Time:  2015-07-13

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef SORT_THREADUNIT
#define	SORT_THREADUNIT

#include "sort.h"

class Integrator;

class ThreadUnit
{
// public method
public:
	// Constructor
	ThreadUnit(){}
	virtual ~ThreadUnit(){}

	// Begin thread
	virtual void BeginThread() = 0;

	// End thread
	virtual void EndThread() = 0;

	// run thread
	virtual void RunThread() = 0;

	// whether the thread is finished
	virtual bool IsFinished() = 0;

// the rendering data
public:
	Integrator*	m_pIntegrator;
};

#endif