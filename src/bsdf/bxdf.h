/*
   FileName:      bxdf.h

   Created Time:  2011-08-04 12:52:25

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_BXDF
#define	SORT_BXDF

// include the header
#include "spectrum/spectrum.h"
#include "utility/enum.h"

// pre-declera class
class Vector;
class BsdfSample;

/////////////////////////////////////////////////////////////////////////
// definition of bxdf
class	Bxdf
{
// public method
public:
	// default constructor
	Bxdf(){}
	// destructor
	virtual ~Bxdf(){}

	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	virtual Spectrum f( const Vector& wo , const Vector& wi ) const = 0;

	// sample a direction randomly
	// para 'wo'  : out going direction
	// para 'wi'  : in direction generated randomly
	// para 'bs'  : bsdf sample variable
	// para 'pdf' : property density function value of the specific 'wi'
	// result     : brdf value for the 'wo' and 'wi'
	virtual Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const;

	// get the pdf of the sampled direction
	// para 'wo' : out going direction
	// para 'wi' : coming in direction from light
	// result    : the pdf for the sample
	virtual float Pdf( const Vector& wi , const Vector& wo ) const;

	// whether the flag is matched
	bool	MatchFlag( BXDF_TYPE type ) const
	{return (type & m_type)==m_type;}

	// get bxdf type
	BXDF_TYPE GetType() const { return m_type; }

	// weight for the bxdf
	Spectrum	m_weight;

// protected field
protected:
	// the type for the bxdf
	BXDF_TYPE m_type;
};

#endif
