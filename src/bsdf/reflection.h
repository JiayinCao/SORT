/*
 * filename :	reflection.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_REFLECTION
#define	SORT_REFLECTION

// include the header
#include "bxdf.h"

////////////////////////////////////////////////////////////////////
// definition of reflection
class	Reflection : Bxdf
{
// public method
public:
	// default constructor
	Reflection(){m_type=BXDF_REFLECTION;}
	// destructor
	~Reflection(){}

	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const ;

	// sample a direction randomly
	// para 'wo'  : out going direction
	// para 'wi'  : in direction generated randomly
	// para 'pdf' : property density function value of the specific 'wi'
	// result     : brdf value for the 'wo' and 'wi'
	virtual Spectrum Sample_f( const Vector& wo , Vector& wi , float* pdf ) const ;
};

#endif
