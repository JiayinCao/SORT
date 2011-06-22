/*
 * filename :	rgbspectrum.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_RGBSPECTRUM
#define	SORT_RGBSPECTRUM

#include "sort.h"

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
	float	GetR() const { return m_r; }
	float	GetG() const { return m_g; }
	float	GetB() const { return m_b; }

	// operators
	RGBSpectrum operator+( const RGBSpectrum& c ) const;
	RGBSpectrum operator-( const RGBSpectrum& c ) const;
	RGBSpectrum operator*( const RGBSpectrum& c ) const;

	RGBSpectrum operator+( float t ) const;
	RGBSpectrum operator-( float t ) const;
	RGBSpectrum operator*( float t ) const;

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
	r = max( 0.0f , r );
	r = min( 1.0f , r );
	float g = t - s.m_g;
	g = max( 0.0f , g );
	g = min( 1.0f , g );
	float b = t - s.m_b;
	b = max( 0.0f , b );
	b = min( 1.0f , b );

	return RGBSpectrum( r , g , b );
}
inline RGBSpectrum operator*( float t , const RGBSpectrum& s )
{
	return s * t;
}

#endif


