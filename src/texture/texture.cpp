/*
 * filename :	texture.cpp
 *
 * programmer : Cao Jiayin
 */

// include the header file
#include "texture.h"
#include <math.h>
#include "../managers/texmanager/texmanager.h"
#include "compositetexture.h"
#include "../managers/logmanager.h"

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
bool Texture::Output( const string& str , TEX_TYPE type )
{	
	// output the texture
	return TexManager::GetSingleton().Write( str , this , type );
}

// get the color
Spectrum Texture::GetColor( float u , float v ) const
{
	return GetColor( (int) (u *  m_iTexWidth) , (int) (v * m_iTexHeight) );
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
			x = m_iTexWidth - ( -x ) % m_iTexWidth;
		if( y >= 0 )
			y = y % m_iTexHeight;
		else
			y = m_iTexHeight - ( -y ) % m_iTexHeight;
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
