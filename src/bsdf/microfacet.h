/*
   FileName:      microfacet.h

   Created Time:  2013-03-17 19:08:55

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MICROFACET
#define	SORT_MICROFACET

// include header file
#include "bxdf.h"
#include "fresnel.h"
#include "microfacetdistribution.h"

/////////////////////////////////////////////////////////////////////
// microfacet bxdf
class MicroFacet : public Bxdf
{
// public method
public:
	// constructor
	MicroFacet(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d );
	
	// evaluate bxdf
	// para 'wo' : out going direction
	// para 'wi' : in direction
	// result    : the portion that comes along 'wo' from 'wi'
	Spectrum f( const Vector& wo , const Vector& wi ) const;
	
// private field
private:
	// reflectance
	Spectrum R;
	// distribution of facets
	MicroFacetDistribution* distribution;
	// fresnel for the surface
	Fresnel* fresnel;
	
// private method
	// geometric attenuation term
	float G(const Vector &wo, const Vector &wi, const Vector &wh) const;
};

#endif
