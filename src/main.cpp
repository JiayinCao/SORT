/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "system.h"
#include "geometry/transform.h"
#include "geometry/triangle.h"
#include "geometry/instancetri.h"
#include "texture/imagetexture.h"
#include "texture/checkboxtexture.h"
#include "managers/texmanager/texmanager.h"

// the global system
System g_System;

// the main func
int main( int argc , char** argv )
{
	CheckBoxTexture cb;
	cb.Output( "tex.bmp" );

	ImageTexture tex , t1 , t2;
	tex.LoadImage( "tex.bmp" );
	t1.LoadImage( "tex.bmp" );
	t2.LoadImage( "tex.bmp" );

	return 0;
}
