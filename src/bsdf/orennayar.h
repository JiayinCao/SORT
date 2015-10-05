/*
   FileName:      orennayar.h

   Created Time:  2013-03-16 23:27:15

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_ORENNAYAR
#define	SORT_ORENNAYAR

// include header file
#include "bxdf.h"

////////////////////////////////////////////////////////////////////////
//	A geometric-optics microfacet-based brdf model that is proposed by
//	Oren and Nayar (1994)
class OrenNayar : public Bxdf
{
// public method
public:
	// constructor
	OrenNayar( const Spectrum& reflectence , float roughness);
	
	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const;
	
// private field
private:
	// spectrum for scaling the brdf
	Spectrum R;
	// the factors
	float	A;
	float	B;
};

#endif