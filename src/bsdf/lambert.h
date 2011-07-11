/*
 * filename :	lambert.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_LAMBERT
#define	SORT_LAMBERT

// include header file
#include "bxdf.h"

////////////////////////////////////////////////////////////
// definition of lambert brdf
class Lambert : public Bxdf
{
// public method
public:
	// default constructor
	Lambert(){ m_type=BXDF_DIFFUSE; }
	// constructor
	// para 's' : reflect density
	Lambert( const Spectrum& s ):R(s){m_type=BXDF_DIFFUSE;}
	// destructor
	~Lambert(){}

	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const;

	// sample a direction randomly
	// para 'wo'  : out going direction
	// para 'wi'  : in direction generated randomly
	// para 'pdf' : property density function value of the specific 'wi'
	// result     : brdf value for the 'wo' and 'wi'
	virtual Spectrum Sample_f( const Vector& wo , Vector& wi , float* pdf ) const;

	// set color
	void SetColor( const Spectrum& color ) { R = color; }

// private field
private:
	// the total reflectance
	Spectrum R;
};

#endif
