/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "system.h"
#include "camera/perspective.h"
#include "utility/timer.h"

// the global system
System g_System;

// the main func
int main( int argc , char** argv )
{
	// pre process before rendering
	g_System.PreProcess();

	Timer::GetSingleton().StartTimer();
	// do ray tracing
	g_System.Render();
	Timer::GetSingleton().StopTimer();
	cout<<Timer::GetSingleton().GetElapsedTime()<<endl;

	// output image
	g_System.OutputRT( "t.bmp" );

	return 0;
}
