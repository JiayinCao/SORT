/*
 * filename :	rendertarget.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_RENDERTARGET
#define	SORT_RENDERTARGET

// include the header file
#include "texture.h"

/////////////////////////////////////////////////////////////////////////
//	defination of render target
//	all of the generated image will be rendered to render target.
class	RenderTarget : public Texture
{
//public method
public:
	// constructor and destructor
	RenderTarget(){ _init(); }
	~RenderTarget(){ Release(); }

	// get color from render target
	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// result   : the cooresponding color
	// note     : the x or y is out of range , use filter to make them in the range.
	//            if there is no data in the render target, there will be a run-time
	//			  and log error.
	const Spectrum& GetColor( int x , int y ) const;

	// set the size for the render target
	// para 'w' : width of the render target
	// para 'y' : height of the render target
	// note     : if any of the two edge is zero length , there will be a warning in
	//			  log. any call to the method will cause allocating memory if parameters
	//			  are both valid.
	void SetSize( unsigned w , unsigned h );

	// release the memory in the render target
	void Release();

	// set the color of the render target
	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// para 'r' : red component
	// para 'g' : green component
	// para 'b' : blue component
	void SetColor( int x , int y , float r , float g , float b );

	// para 'x' : x coordinate
	// para 'y' : y coordinate
	// para 'c' : color in spectrum form
	void SetColor( int x , int y , const Spectrum& c )
	{
		SetColor( x , y , c.GetR() , c.GetG() , c.GetB() );
	}

//private field
private:
	// the data pointer
	Spectrum*	m_pData;

//private method
	// initialize the data
	void	_init();
};

#endif
