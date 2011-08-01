/*
 * filename :	main.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "system.h"
#include "utility/samplemethod.h"
#include "texture/imagetexture.h"
#include "texture/rendertarget.h"

// the global system
System g_System;

// the main func
int main( int argc , char** argv )
{
	ImageTexture image;
	image.SetProperty( "filename" , "../res/test.bmp" );
	
	Distribution2D dis( &image );
	unsigned nu = image.GetWidth();
	unsigned nv = image.GetHeight();

	RenderTarget rt;
	rt.SetSize( image.GetWidth() , image.GetHeight() );

	srand( time(0) );
	for( int i = 0 ; i < 1000000; i++ )
	{
		float u = ((float) ( rand() % 1024 ) / 1024.0f);
		float v = ((float) ( rand() % 1024 ) / 1024.0f);

		int t = rand() % 4;
		while( t-- > 0 )
			v = (float) (rand() % 1024 ) / 1024.0f;

		float uv[2];
		dis.SampleContinuous( u , v , uv , 0 );

		int tu = uv[0] * nu;
		int tv = uv[1] * nv;
		Spectrum c = rt.GetColor(tu,tv);
		c+=0.1f;

		rt.SetColor(tu,tv,c);
	}

	rt.Output( "tt.bmp" );

	return 0;

	// load the scene from file
	g_System.LoadScene( "../res/first_scene.scene" );

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
