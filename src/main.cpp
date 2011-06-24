/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "system.h"
#include "geometry/trimesh.h"
#include "camera/perspective.h"
#include "utility/timer.h"
#include "utility/path.h"

// the global system
System g_System;

// the main func
int main( int argc , char** argv )
{
	PerspectiveCamera* camera = new PerspectiveCamera();
	camera->SetEye( Point( 50 , 50 , 50 ) );
	camera->SetUp( Vector( 0 , 1 , 0 ) );
	camera->SetTarget( Point( 0 , 10 , 0 ) );
	camera->SetFov( 3.1415f / 4 );

	camera->SetRenderTarget( g_System.m_rt );
	g_System.m_camera = camera;

	g_System.LoadScene( "" );

	Timer::GetSingleton().StartTimer();

	g_System.Render();

	Timer::GetSingleton().StopTimer();
	cout<<Timer::GetSingleton().GetElapsedTime()<<endl;

	g_System.OutputRT( "t.bmp" );

	return 0;
}
