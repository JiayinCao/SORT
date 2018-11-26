/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

// include the header
#include "rgbspectrum.h"
#include "utility/define.h"
#include "material/material_node.h"

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

	color |= ((unsigned char)(255.0f*saturate(m_r)))<<16;
	color |= ((unsigned char)(255.0f*saturate(m_g)))<<8;
	color |= ((unsigned char)(255.0f*saturate(m_b)))<<0;

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

// / operator
RGBSpectrum RGBSpectrum::operator / ( const RGBSpectrum& c ) const
{
	float r = m_r / c.m_r;
	float g = m_g / c.m_g;
	float b = m_b / c.m_b;

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

// / operator
RGBSpectrum RGBSpectrum::operator / ( float t ) const
{
	float r = m_r / t;
	float g = m_g / t;
	float b = m_b / t;

	return RGBSpectrum( r , g , b );
}

// get each component
float RGBSpectrum::GetR() const { 
	return m_r;
}
float RGBSpectrum::GetG() const { 
	return m_g;
}
float RGBSpectrum::GetB() const { 
	return m_b;
}
float RGBSpectrum::GetMaxComponent() const{
    return std::max( m_r , std::max( m_b , m_g ) );
}

// clamp the spectrum
RGBSpectrum RGBSpectrum::Clamp( float low , float high ) const
{
	return RGBSpectrum( clamp(m_r, low, high) , clamp(m_g, low, high) , clamp(m_b, low, high) );
}
