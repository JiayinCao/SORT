/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "texture/gridtexture.h"
#include "utility/timer.h"

// the main func
int main( int argc , char** argv )
{
	// initialize sort first
	initSort();
	GridTexture tex1;
	GridTexture com;

	Timer::GetSingleton().StartTimer();

	for( int i = 0 ; i < 1000000; i++ )
		for( int j = 0; j < 1; j++ )
		{
			Spectrum c = tex1.GetColor( 0 , 0 ) * com.GetColor( 0 , 0 );
		}

	Timer::GetSingleton().StopTimer();

	cout<<Timer::GetSingleton().GetElapsedTime()<<endl;

	// release sort
	releaseSort();

	return 0;
}
