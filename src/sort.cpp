/*
 * filename  :	sort.cpp
 *
 * programmer:	Cao Jiayin
 */

// include the header file
#include <iostream>
#include "texture/gridtexture.h"
#include "managers/texmanager/texmanager.h"

// use standard namespace
using namespace std;

// the enter point for the program
int main( int argc , char** argv )
{
	TexManager::CreateTexManager();

	GridTexture tex( 0.0f , 0.0f , 1.0f , 0.0f , 0.0f , 0.0f );
	tex.SetSize( 128 , 128 );
	tex.SetTexCoordFilter( TCF_MIRROR );
	tex.Output( "hello.bmp" );

	return 0;
} 
