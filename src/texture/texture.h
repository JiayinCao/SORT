/*
 * filename :	texture.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef SORT_TEXTURE
#define SORT_TEXTURE

// include header file
#include "../sort.h"
#include "../spectrum/spectrum.h"
#include "../utility/enum.h"

// pre-declare class
class ComTexture;

//////////////////////////////////////////////////////////////
// defination of class Texture
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
	// para 'type':	the way outputing the texture , default way is saving the texture as a bitmap file
	// result : return true , if outputing is successful
	bool Output( const string& str , TEX_TYPE type = TT_BMP );

	// get the texture value
	// para 'x' :	x coordinate , if out of range , use filter
	// para 'y' :	y coordinate , if out of range , use filter
	// result	:	spectrum value in the position
	virtual const Spectrum& GetColor( int x , int y ) const = 0;

	// get the texture value
	// para 'u'	:	x coordinate , if out of range , use filter
	// para 'v' :	y coordinate , if out of range , use filter
	virtual const Spectrum& GetColor( float u , float v ) const;

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
	ComTexture operator + ( const Texture& tex );
	ComTexture operator - ( const Texture& tex );
	ComTexture operator * ( const Texture& tex );

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
};

#endif

