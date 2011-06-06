/*
 * filename :	rgbspectrum.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_RGBSPECTRUM
#define	SORT_RGBSPECTRUM

///////////////////////////////////////////////////////////////////
// defination of rgb spectrum
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
	// destructor
	~RGBSpectrum();

	// get the color
	unsigned int GetColor() const;

// private field
private:
	// the rgb color
	float m_r;
	float m_g;
	float m_b;
};

#endif


