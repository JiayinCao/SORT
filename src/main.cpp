/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "texture/imagetexture.h"
#include "managers/logmanager.h"
#include "utility/error.h"
#include "geometry/vector.h"

// the main func
int main( int argc , char** argv )
{
	// initialize sort first
	initSort();

	// create a grid texture and save it
	ImageTexture tex;
	tex.LoadImage( "tex.bmp" );
	tex.Output( "tex1.bmp" );

	// release sort
	releaseSort();

	return 0;
}
