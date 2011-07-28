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
#include "light/light.h"

// radiance along a specific ray direction
Spectrum WhittedRT::Li( const Scene& scene , const Ray& r ) const
{
	if( r.m_Depth > 6 )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return scene.EvaluateSky( r );

	Spectrum t;

	// get bsdf
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );

	// lights
	const vector<Light*>& lights = scene.GetLights();
	vector<Light*>::const_iterator it = lights.begin();
	while( it != lights.end() )
	{
		Vector	lightDir;
		float	pdf;
		Spectrum ld = (*it)->sample_f( ip , lightDir , &pdf );
		Spectrum f = bsdf->f( -r.m_Dir , lightDir );
		t += (ld * f * SatDot( lightDir , ip.normal ) / pdf);
		it++;
	}

	// add reflection
	if( bsdf->NumComponents( BXDF_REFLECTION ) > 0 )
		t += SpecularReflection( scene , r , &ip , bsdf , this );
	if( bsdf->NumComponents( BXDF_TRANSMISSION ) > 0 )
		t += SpecularRefraction( scene , r , &ip , bsdf , this );

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
