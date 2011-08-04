/*
   FileName:      skysphere.cpp

   Created Time:  2011-08-04 12:51:50

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "skysphere.h"
#include "bsdf/bsdf.h"
#include "geometry/ray.h"

// default constructor
SkySphere::SkySphere()
{
	_registerAllProperty();
}

// evaluate value from sky
Spectrum SkySphere::Evaluate( const Ray& r ) const
{
	float theta = SphericalTheta( r.m_Dir );
	float phi = SphericalPhi( r.m_Dir );

	float v = theta * 2 * INV_PI;
	float u = phi * INV_PI;

	return m_sky.GetColor( u , v );
}

// register property
void SkySphere::_registerAllProperty()
{
	_registerProperty( "image" , new ImageProperty( this ) );
}
