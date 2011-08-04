/*
   FileName:      texture.cpp

   Created Time:  2011-08-04 12:45:36

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "texture.h"
#include <math.h>
#include "managers/texmanager.h"
#include "compositetexture.h"
#include "managers/logmanager.h"
#include "geometry/intersection.h"

// default constructor
Texture::Texture()
{
	// inintialize default value
	_init();
}

// destructor
Texture::~Texture()
{
	m_iTexWidth = 0;
	m_iTexHeight = 0;
}

// initialize default value
void Texture::_init()
{
	m_iTexWidth = 0;
	m_iTexHeight = 0;
	m_TexCoordFilter = TCF_WARP;
}

// output the texture
bool Texture::Output( const string& str )
{	
	// output the texture
	return TexManager::GetSingleton().Write( str , this );
}

// get the color
Spectrum Texture::Evaluate( const Intersection* intersect ) const
{
	return GetColor( (int) (intersect->u *  m_iTexWidth) , (int) (intersect->v * m_iTexHeight) );
}

// set texture coordinate filter mode
void Texture::SetTexCoordFilter( TEXCOORDFILTER mode )
{
	m_TexCoordFilter = mode;
}

// do texture coordinate filter
void Texture::_texCoordFilter( int& x , int& y ) const
{
	switch( m_TexCoordFilter )
	{
	case TCF_WARP:
		if( x >= 0 ) 
			x = x % m_iTexWidth;
		else
			x = m_iTexWidth - ( -x ) % m_iTexWidth - 1;
		if( y >= 0 )
			y = y % m_iTexHeight;
		else
			y = m_iTexHeight - ( -y ) % m_iTexHeight - 1;
		break;
	case TCF_CLAMP:
		x = min( (int)m_iTexWidth - 1 , max( x , 0 ) );
		y = min( (int)m_iTexHeight - 1 , max( y , 0 ) );
		break;
	case TCF_MIRROR:
		x = ( x >= 0 )?x:(1-x);
		x = x % ( 2 * m_iTexWidth );
		x -= m_iTexWidth;
		x = ( x >= 0 )?x:(1-x);
		x = m_iTexWidth - 1 - x;
		y = ( y >= 0 )?y:(1-y);
		y = y % ( 2 * m_iTexHeight );
		y -= m_iTexHeight;
		y = ( y >= 0 )?y:(1-y);
		y = m_iTexHeight - 1 - y;
		break;
	}
}

// the operator
ComTexture Texture::operator + ( const Texture& tex ) const
{
	if( tex.GetWidth() != m_iTexWidth || tex.GetHeight() != m_iTexHeight )
		LOG_ERROR<<"Size of the images are not the same , can't add together."<<CRASH;

	if( m_iTexWidth == 0 || m_iTexHeight == 0 )
		LOG_ERROR<<"One dimension of the image is zero , can't add together."<<CRASH;

	//allocate the data
	Spectrum* data = new Spectrum[ m_iTexWidth * m_iTexHeight ];

	for( unsigned i = 0 ; i < m_iTexHeight; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			data[offset] = tex.GetColor( (int)j , (int)i ) + GetColor( (int)j , (int)i );
		}

	return ComTexture( data , m_iTexWidth , m_iTexHeight );
}

// the operator
ComTexture Texture::operator - ( const Texture& tex ) const
{
	if( tex.GetWidth() != m_iTexWidth || tex.GetHeight() != m_iTexHeight )
		LOG_ERROR<<"Size of the images are not the same , can't substract."<<CRASH;

	if( m_iTexWidth == 0 || m_iTexHeight == 0 )
		LOG_ERROR<<"One dimension of the image is zero , can't substract."<<CRASH;

	//allocate the data
	Spectrum* data = new Spectrum[ m_iTexWidth * m_iTexHeight ];

	for( unsigned i = 0 ; i < m_iTexHeight; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			data[offset] = GetColor( (int)j , (int)i ) - tex.GetColor( (int)j , (int)i );
		}

	return ComTexture( data , m_iTexWidth , m_iTexHeight );
}

// the operator
ComTexture Texture::operator * ( const Texture& tex ) const
{
	if( tex.GetWidth() != m_iTexWidth || tex.GetHeight() != m_iTexHeight )
		LOG_ERROR<<"Size of the images are not the same , can't multiply."<<CRASH;

	if( m_iTexWidth == 0 || m_iTexHeight == 0 )
		LOG_ERROR<<"One dimension of the image is zero , can't multiply."<<CRASH;

	//allocate the data
	Spectrum* data = new Spectrum[ m_iTexWidth * m_iTexHeight ];

	for( unsigned i = 0 ; i < m_iTexHeight; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			data[offset] = tex.GetColor( (int)j , (int)i ) * GetColor( (int)j , (int)i );
		}

	return ComTexture( data , m_iTexWidth , m_iTexHeight );
}

// the operator
ComTexture Texture::operator + ( float t ) const
{
	if( m_iTexWidth == 0 || m_iTexHeight == 0 )
		LOG_ERROR<<"One dimension of the image is zero , can't add together."<<CRASH;

	//allocate the data
	Spectrum* data = new Spectrum[ m_iTexWidth * m_iTexHeight ];

	for( unsigned i = 0 ; i < m_iTexHeight; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			data[offset] = GetColor( (int)j , (int)i ) + t;
		}

	return ComTexture( data , m_iTexWidth , m_iTexHeight );
}

// the operator
ComTexture Texture::operator - ( float t ) const
{
	if( m_iTexWidth == 0 || m_iTexHeight == 0 )
		LOG_ERROR<<"One dimension of the image is zero , can't substract."<<CRASH;

	//allocate the data
	Spectrum* data = new Spectrum[ m_iTexWidth * m_iTexHeight ];

	for( unsigned i = 0 ; i < m_iTexHeight; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			data[offset] = GetColor( (int)j , (int)i ) - t;
		}

	return ComTexture( data , m_iTexWidth , m_iTexHeight );
}

// the operator
ComTexture Texture::operator * ( float t ) const
{
	if( m_iTexWidth == 0 || m_iTexHeight == 0 )
		LOG_ERROR<<"One dimension of the image is zero , can't multiply."<<CRASH;

	//allocate the data
	Spectrum* data = new Spectrum[ m_iTexWidth * m_iTexHeight ];

	for( unsigned i = 0 ; i < m_iTexHeight; i++ )
		for( unsigned j = 0 ; j < m_iTexWidth ; j++ )
		{
			unsigned offset = i * m_iTexWidth + j;
			data[offset] = GetColor( (int)j , (int)i ) * t;
		}

	return ComTexture( data , m_iTexWidth , m_iTexHeight );
}

ComTexture operator+( float t , const Texture& tex )
{
	return tex + t;
}

ComTexture operator-( float t , const Texture& tex )
{
	if( tex.m_iTexWidth == 0 || tex.m_iTexHeight == 0 )
		LOG_ERROR<<"One dimension of the image is zero , can't multiply."<<CRASH;

	//allocate the data
	Spectrum* data = new Spectrum[ tex.m_iTexWidth * tex.m_iTexHeight ];

	for( unsigned i = 0 ; i < tex.m_iTexHeight; i++ )
		for( unsigned j = 0 ; j < tex.m_iTexWidth ; j++ )
		{
			unsigned offset = i * tex.m_iTexWidth + j;
			data[offset] = t - tex.GetColor( (int)j , (int)i );
		}

	return ComTexture( data , tex.m_iTexWidth , tex.m_iTexHeight );
}

ComTexture operator*( float t , const Texture& tex )
{
	return tex * t;
}
