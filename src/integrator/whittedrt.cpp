/*
 * filename :	whittedrt.cpp
 *
 * programmer :	Cao Jiayin
 */

// include the header file
#include "whittedrt.h"
#include "integratormethod.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"

// radiance along a specific ray direction
Spectrum WhittedRT::Li( const Scene& scene , const Ray& r ) const
{
	Intersection ip;
	if( scene.GetIntersect( r , &ip ) )
		return Spectrum( (float)fabs( ip.normal.x ) , (float)fabs( ip.normal.y ) , (float)fabs( ip.normal.z ) );

	return Spectrum();
}
