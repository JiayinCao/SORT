/*
   FileName:      reflection.h

   Created Time:  2011-08-04 12:52:46

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_REFLECTION
#define	SORT_REFLECTION

// include the header
#include "bxdf.h"
#include "fresnel.h"

////////////////////////////////////////////////////////////////////
// definition of reflection
class	Reflection : public Bxdf
{
// public method
public:
	// default constructor
	Reflection( const Fresnel* fre ):
		m_fresnel(fre)
	{m_type=BXDF_REFLECTION;}
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
	virtual Spectrum sample_f( const Vector& wo , Vector& wi , float* pdf ) const ;

// private field
private:
	const Fresnel* m_fresnel;
};

#endif
