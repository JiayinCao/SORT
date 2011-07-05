/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "system.h"
#include "utility/strhelper.h"

// the global system
System g_System;

// the main func
int main( int argc , char** argv )
{
	// load the scene from file
	g_System.LoadScene("../res/first_scene.scene");

	// do ray tracing
	g_System.Render();

	// output rendering time
	cout<<g_System.GetRenderingTime()<<endl;

	// output log information
	g_System.OutputLog();

	// output image
	g_System.OutputRT( "t.bmp" );

	return 0;
}
