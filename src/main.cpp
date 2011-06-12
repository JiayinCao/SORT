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
#include "texture/rendertarget.h"

// the main func
int main( int argc , char** argv )
{
	// initialize sort first
	initSort();

	// create a grid texture and save it
	GridTexture tex0( 1 , 0 , 0 , 0 , 0 , 0 );
	tex0.SetSize( 128 , 128 );
	tex0.Output( "test1.bmp" );

	ImageTexture tex1;
	tex1.LoadImage( "test1.bmp" );
	tex1.Output( "t.bmp" );

	// a render target
	RenderTarget target;

	unsigned w = tex1.GetWidth();
	unsigned h = tex1.GetHeight();
	target.SetSize( w , h );
	for( unsigned i = 0 ; i < w ; i++ )
	{
		for( unsigned j = 0 ; j < h ; j++ )
		{
			const Spectrum& color = tex1.GetColor( i , j );
			target.SetColor( i , j , color );
		}
	}
	target.Output( "target.bmp" );

	ComTexture com = target;
	com.Output( "tt.bmp" );

	GridTexture tex2( 0,1,0,1,0,1 );
	tex2.SetSize( 128 , 128 );
	com = tex2 * target + tex0;
	com.Output( "tex2.bmp" );

	// release sort
	releaseSort();

	return 0;
}
