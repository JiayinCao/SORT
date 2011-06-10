/*
 * filename :	rgbspectrum.cpp
 *
 * programmer : Cao Jiayin
 */

// include the header
#include "rgbspectrum.h"

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
	m_r = r;
	m_g = g;
	m_b = b;
}

// constructor from three unsigned char
RGBSpectrum::RGBSpectrum( unsigned char r , unsigned char g , unsigned char b )
{
	m_r = (float)r / 255.0f;
	m_g = (float)g / 255.0f;
	m_b = (float)b / 255.0f;
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