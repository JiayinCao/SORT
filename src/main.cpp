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

// the global system
System g_System;

// the main func
int main( int argc , char** argv )
{
	TriMesh mesh;
	mesh.LoadMesh( "../res/cube.obj" );

	PerspectiveCamera* camera = new PerspectiveCamera();
	camera->SetEye( Point( 3 , 3 , 3 ) );
	camera->SetUp( Vector( 0 , 1 , 0 ) );
	camera->SetTarget( Point( 0 , 0 , 0 ) );
	camera->SetFov( 3.1415f / 4 );

	camera->SetRenderTarget( g_System.m_rt );
	g_System.m_camera = camera;

	Timer::GetSingleton().StartTimer();

	for( int i = 0 ; i < 10 ; i++ )
		g_System.Render( &mesh );

	Timer::GetSingleton().StopTimer();
	cout<<Timer::GetSingleton().GetElapsedTime()/10<<endl;

	g_System.OutputRT( "t.bmp" );
	
	return 0;
}
