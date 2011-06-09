/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "texture/gridtexture.h"
#include "managers/logmanager.h"
#include "utility/error.h"
#include "geometry/vector.h"

// the main func
int main( int argc , char** argv )
{
	// initialize sort first
	initSort();

	// create a grid texture and save it
	GridTexture tex( 1.0f , 0.0f , 0.0f , 0.0f , 1.0f , 0.0f );
	tex.SetSize( 128 , 128 );
	tex.Output( "tex.bmp" );

	// release sort
	releaseSort();

	return 0;
}
