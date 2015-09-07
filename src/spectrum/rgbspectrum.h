/*
   FileName:      rgbspectrum.h

   Created Time:  2011-08-04 12:46:14

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_RGBSPECTRUM
#define	SORT_RGBSPECTRUM

#include "sort.h"
#include "utility/define.h"

struct MaterialPropertyValue;

///////////////////////////////////////////////////////////////////
// definition of rgb spectrum
class	RGBSpectrum
{
// public method
public:
	// default constructor
	RGBSpectrum();
	// constructor from node property
	RGBSpectrum( const MaterialPropertyValue& prop );
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

	// clamp the spectrum
	RGBSpectrum Clamp() const;

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
        const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
        return YWeight[0] * m_r + YWeight[1] * m_g + YWeight[2] * m_b;
	}

// private field
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

#endif


