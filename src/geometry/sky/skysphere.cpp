/*
 * filename :	skysphere.cpp
 *
 * programmer :	Cao Jiayin
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

	float u = theta * 2 * INV_PI;
	float v = -phi * INV_PI;

	return m_sky.GetColor( u , v );
}

// register property
void SkySphere::_registerAllProperty()
{
	_registerProperty( "image" , new ImageProperty( this ) );
}