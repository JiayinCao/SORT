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

// radiance along a specific ray direction
Spectrum WhittedRT::Li( const Scene& scene , const Ray& r ) const
{
	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.EvaluateSky( r );

	// get light direction ( not implemented )
	Vector lightDir = Vector( 2.0f , 5.0f , 3.0f );
	lightDir.Normalize();
	Spectrum lightdensity = Spectrum( 20.0f );

	float density = max( 0.0f , Dot( lightDir , ip.normal ) );

	// get primitive bsdf
	Spectrum t;
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );
	Spectrum c = bsdf->f( -r.m_Dir , lightDir );
	t = c * density * lightdensity;

	// add reflection
	if( Dot( -r.m_Dir , ip.normal ) > 0.01f )
	{
		if( bsdf->NumComponents( BXDF_REFLECTION ) > 0 )
			t += SpecularReflection( scene , r , &ip , bsdf , this );
		if( bsdf->NumComponents( BXDF_TRANSMISSION ) > 0 )
			t += SpecularRefraction( scene , r , &ip , bsdf , this );
	}

	return t;
}

// output log information
void WhittedRT::OutputLog() const
{
	LOG_HEADER( "Integrator" );
	LOG<<"Integrator algorithm : Whitted Ray Tracing."<<ENDL;
	LOG<<"It supports direct lighting , specular reflection and specular refraction."<<ENDL;
	LOG<<"Indirect lighting , like color bleeding , caustics , is not supported."<<ENDL<<ENDL;
}
