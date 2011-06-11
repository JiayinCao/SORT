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
#include "texture/gridtexture.h"
#include "geometry/vector.h"

// the main func
int main( int argc , char** argv )
{
	// initialize sort first
	initSort();

	// create a grid texture and save it
	GridTexture tex0;
	tex0.Output( "test1.bmp" );

	ImageTexture tex1;
	tex1.LoadImage( "te.bmp" );
	tex1.Output( "t.bmp" );

	// release sort
	releaseSort();

	return 0;
}
