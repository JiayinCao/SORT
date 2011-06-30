/*
 * filename :	texture.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_TEXTURE
#define SORT_TEXTURE

// include header file
#include "sort.h"
#include "spectrum/spectrum.h"
#include "utility/enum.h"

// pre-declare class
class ComTexture;
class Intersection;

//////////////////////////////////////////////////////////////
// definition of class Texture
class Texture
{
// public method
public:
	// default constructor
	Texture();
	// destructor
	virtual ~Texture();

	// output the texture
	// para 'str' :	the name of the output entity
	// result : return true , if outputing is successful
	bool Output( const string& str );

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual Spectrum GetColor( int x , int y ) const = 0;

	// whether the texture support outputing
	// result : 'true' if the texture supports texture output
	virtual	bool CanOutput() const { return true; }

	// get the texture value
	// para 'intersect' : the intersection
	// result :	the spectrum value
	virtual Spectrum Evaluate( const Intersection* intersect ) const;

	// set texture coordinate filter mode
	void	SetTexCoordFilter( TEXCOORDFILTER mode );

	// get the size of the texture
	unsigned	GetWidth() const
	{
		return m_iTexWidth;
	}
	unsigned	GetHeight() const
	{
		return m_iTexHeight;
	}

	// set the size of the texture
	virtual void	SetSize( unsigned w , unsigned h )
	{
		m_iTexWidth = w;
		m_iTexHeight = h;
	}

	// texture supports + - * operator for blending
	ComTexture operator + ( const Texture& tex ) const ;
	ComTexture operator - ( const Texture& tex ) const ;
	ComTexture operator * ( const Texture& tex ) const ;

	ComTexture operator + ( float ) const;
	ComTexture operator - ( float ) const;
	ComTexture operator * ( float ) const;

// protected field
protected:
	// the size of the texture
	// default values are zero
	unsigned	m_iTexWidth;
	unsigned	m_iTexHeight;

	// texture coordinate filter mode
	// default value is warp
	TEXCOORDFILTER	m_TexCoordFilter;

// protected method
protected:
	// initialize default value for texture
	void _init();

	// do texture filter
	void _texCoordFilter( int& u , int&v ) const;

// set friend class
friend ComTexture operator-( float t , const Texture& tex );

};

ComTexture operator+( float t , const Texture& tex );
ComTexture operator*( float t , const Texture& tex );
ComTexture operator-( float t , const Texture& tex );

#endif

