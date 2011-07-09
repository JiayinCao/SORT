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
#include "bsdf/bsdf.h"

// to be deleted
#include "bsdf/bxdf.h"
#include "bsdf/lambert.h"

// radiance along a specific ray direction
Spectrum WhittedRT::Li( const Scene& scene , const Ray& r ) const
{
	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return Spectrum();

	// get light direction ( not implemented )
	Vector lightDir = Vector( -1.0f , 5.0f , 7.0f );
	lightDir.Normalize();

	float density = max( 0.0f , Dot( lightDir , ip.normal ) );
	if( density == 0.0f )
		return Spectrum();

	// get primitive bsdf
	Spectrum t = Spectrum ( 1.0f , 1.0f , 1.0f );
	if( ip.primitive->GetMaterial() )
	{
		Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );
		if( bsdf )
			t = bsdf->f( -r.m_Dir , lightDir );
	}

	return t * density; 
}

// output log information
void WhittedRT::OutputLog() const
{
	LOG_HEADER( "Integrator" );
	LOG<<"Integrator algorithm : Whitted Ray Tracing."<<ENDL;
	LOG<<"It supports direct lighting , specular reflection and specular refraction."<<ENDL;
	LOG<<"Indirect lighting , like color bleeding , caustics , is not supported."<<ENDL<<ENDL;
}
