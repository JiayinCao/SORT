/*
 * filename :	rgbspectrum.cpp
 *
 * programmer : Cao Jiayin
 */

// include the header
#include "rgbspectrum.h"

#define saturate(x) max(0.0f,min(1.0f,x))

// default constructor
RGBSpectrum::RGBSpectrum()
{
	m_r = 0.0f;
	m_g = 0.0f;
	m_b = 0.0f;
}

// constructor from three float
RGBSpectrum::RGBSpectrum( float r , float g , float b )
{
	m_r = saturate(r);
	m_g = saturate(g);
	m_b = saturate(b);
}

// constructor from three unsigned char
RGBSpectrum::RGBSpectrum( unsigned char r , unsigned char g , unsigned char b )
{
	m_r = (float)r / 255.0f;
	m_g = (float)g / 255.0f;
	m_b = (float)b / 255.0f;
}

// constructor from one float
RGBSpectrum::RGBSpectrum( float g )
{
	m_r = g;
	m_g = g;
	m_b = g;
}

// constructor from one unsigned
RGBSpectrum::RGBSpectrum( unsigned char g )
{
	m_r = (( float ) g ) / 255.0f;
	m_g = m_r;
	m_b = m_r;
}

// destructor
RGBSpectrum::~RGBSpectrum()
{
}

// get the color
unsigned int RGBSpectrum::GetColor() const
{
	unsigned int color = 0;

	color |= ((unsigned char)(255.0f*m_r))<<16;
	color |= ((unsigned char)(255.0f*m_g))<<8;
	color |= ((unsigned char)(255.0f*m_b))<<0;

	return color;
}

// set the color
void RGBSpectrum::SetColor( unsigned int color )
{
	m_r = ((float)( ( color >> 16 ) & 255 )) / 255.0f;
	m_g = ((float)( ( color >> 8 ) & 255 )) / 255.0f;
	m_b = ((float)( ( color >> 0 ) & 255 )) / 255.0f;
}

// + operator
RGBSpectrum RGBSpectrum::operator + ( const RGBSpectrum& c ) const
{
	float r = m_r + c.m_r;
	float g = m_g + c.m_g;
	float b = m_b + c.m_b;

	return RGBSpectrum( r , g , b );
}

// - operator
RGBSpectrum RGBSpectrum::operator - ( const RGBSpectrum& c ) const
{
	float r = m_r - c.m_r;
	float g = m_g - c.m_g;
	float b = m_b - c.m_b;

	return RGBSpectrum( r , g , b );
}

// * operator
RGBSpectrum RGBSpectrum::operator * ( const RGBSpectrum& c ) const
{
	float r = m_r * c.m_r;
	float g = m_g * c.m_g;
	float b = m_b * c.m_b;

	return RGBSpectrum( r , g , b );
}

// + operator
RGBSpectrum RGBSpectrum::operator + ( float t ) const
{
	float r = t + m_r;
	float g = t + m_g;
	float b = t + m_b;

	return RGBSpectrum( r , g , b );
}

// - operator
RGBSpectrum RGBSpectrum::operator - ( float t ) const
{
	float r = m_r - t ;
	float g = m_g - t ;
	float b = m_b - t ;

	return RGBSpectrum( r , g , b );
}

// + operator
RGBSpectrum RGBSpectrum::operator * ( float t ) const
{
	float r = t * m_r;
	float g = t * m_g ;
	float b = t * m_b ;

	return RGBSpectrum( r , g , b );
}


