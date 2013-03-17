/*
   FileName:      microfacetdistribution.cpp

   Created Time:  2013-03-17 19:37:41

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in
                linux and windows , g++ or visual studio 2008 is required.
*/

// include header file
#include "microfacetdistribution.h"
#include "bsdf.h"

// probabilty of facet with specific normal (v)
float Blinn::D(const Vector& v) const
{
	float cos = AbsCosTheta(v);
	return (exp+2.0f) * INV_TWOPI * powf( cos , exp );
}

// probabilty of facet with specific normal (v)
float Anisotropic::D(const Vector& wh) const
{
	float costhetah = AbsCosTheta(wh);
	float d = 1.f - costhetah * costhetah;
	if (d == 0.f) return 0.f;
	float e = (m_ex * wh.x * wh.x + m_ey * wh.y * wh.y) / d;
	return sqrtf((m_ex+2.f) * (m_ey+2.f)) * INV_TWOPI * powf(costhetah, e);
}