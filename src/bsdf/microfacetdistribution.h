/*
   FileName:      microfacetdistribution.h

   Created Time:  2013-03-17 19:15:39

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_MICROFACETDISTRIBUTION
#define	SORT_MICROFACETDISTRIBUTION

#include "geometry/vector.h"

class MicroFacetDistribution
{
// public method
public:
	// probabilty of facet with specific normal (v)
	virtual float D(const Vector& v) const = 0;
};

class Blinn : public MicroFacetDistribution
{
// public method
public:
	// constructor
	Blinn( float e ) { exp = e; }
	
	// probabilty of facet with specific normal (v)
	float D(const Vector& v) const;
	
// private field
private:
	// the exponent
	float exp;
};

class Anisotropic : public MicroFacetDistribution
{
// public method
public:
	// constructor
	Anisotropic( float ei , float et ){
		m_ex = ei; m_ex = et;
		if( m_ex > 10000.0f ) m_ex = 10000.0f;
		if( m_ey > 10000.0f ) m_ey = 10000.0f;
	}
	
	// probabilty of facet with specific normal (v)
	float D(const Vector& v) const;
	
// private field
private:
	float m_ex;
	float m_ey;
};

#endif
