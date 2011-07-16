/*
 * filename :	rgbspectrum.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_RGBSPECTRUM
#define	SORT_RGBSPECTRUM

#include "sort.h"
#include "utility/define.h"

///////////////////////////////////////////////////////////////////
// definition of rgb spectrum
class	RGBSpectrum
{
// public method
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


