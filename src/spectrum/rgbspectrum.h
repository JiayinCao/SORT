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

#pragma once

#include "sort.h"
#include "utility/define.h"
#include <math.h>

///////////////////////////////////////////////////////////////////
// definition of rgb spectrum
class	RGBSpectrum
{
public:
	// default constructor
	RGBSpectrum();
	// constructor from three float
	RGBSpectrum( float r , float g , float b );
	// constructor from three unsigned char
	RGBSpectrum( unsigned char r , unsigned char g , unsigned char b );
	// constructor from one float
	RGBSpectrum( float g );
	// constructor from and unsigned char
	RGBSpectrum( unsigned char g );
	// destructor
	~RGBSpectrum();

	// get the color
	unsigned int GetColor() const;
	// set the color
	void SetColor( unsigned int color );
	void SetColor( float r , float g , float b )
	{
		m_r = r; m_g = g ; m_b = b;
	}
	// get each component
	float	GetR() const; 
	float	GetG() const; 
	float	GetB() const;
    float   GetMaxComponent() const;

	// clamp the spectrum
	RGBSpectrum Clamp( float low = 0.0f , float high = 0.0f ) const;

	// operators
	RGBSpectrum operator+( const RGBSpectrum& c ) const;
	RGBSpectrum operator-( const RGBSpectrum& c ) const;
	RGBSpectrum operator*( const RGBSpectrum& c ) const;
	RGBSpectrum operator/( const RGBSpectrum& c ) const;

	RGBSpectrum operator+( float t ) const;
	RGBSpectrum operator-( float t ) const;
	RGBSpectrum operator*( float t ) const;
	RGBSpectrum operator/( float t ) const;

	RGBSpectrum& operator+= ( const RGBSpectrum& c ) { *this = *this + c ; return *this; }
	RGBSpectrum& operator-= ( const RGBSpectrum& c ) { *this = *this - c ; return *this; }
	RGBSpectrum& operator*= ( const RGBSpectrum& c ) { *this = *this * c ; return *this; }
	RGBSpectrum& operator/= ( const RGBSpectrum& c ) { *this = *this / c ; return *this; }

	RGBSpectrum& operator+=( float t ){ *this = *this + t ; return *this; }
	RGBSpectrum& operator-=( float t ){ *this = *this - t ; return *this; }
	RGBSpectrum& operator*=( float t ){ *this = *this * t ; return *this; }
	RGBSpectrum& operator/=( float t ){ *this = *this / t ; return *this; }

	//whether the spectrum is black
	bool IsBlack() const
	{
		if( m_r > 0.0f )
			return false;
		if( m_g > 0.0f )
			return false;
		if( m_b > 0.0f )
			return false;
		return true;
	}

	// get intensity
	float GetIntensity() const
	{
        static const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
        return YWeight[0] * m_r + YWeight[1] * m_g + YWeight[2] * m_b;
	}
    
    void ToLinear(){
        m_r = GammaToLinear(m_r);
        m_g = GammaToLinear(m_g);
        m_b = GammaToLinear(m_b);
    }
    void ToGamma(){
        m_r = LinearToGamma(m_r);
        m_g = LinearToGamma(m_g);
        m_b = LinearToGamma(m_b);
    }

    RGBSpectrum Exp() const {
        return RGBSpectrum((float)::exp(m_r), (float)::exp(m_g), (float)::exp(m_b));
    }

private:
	// the rgb color
	float m_r;
	float m_g;
	float m_b;

// set friend function
friend inline RGBSpectrum operator-( float t , const RGBSpectrum& s );
};

// global operator for spectrum
inline RGBSpectrum operator+( float t , const RGBSpectrum& s )
{
	return s + t;
}
inline RGBSpectrum operator-( float t , const RGBSpectrum& s )
{
	float r = t - s.m_r;
	float g = t - s.m_g;
	float b = t - s.m_b;

	return RGBSpectrum( r , g , b );
}
inline RGBSpectrum operator*( float t , const RGBSpectrum& s )
{
	return s * t;
}
