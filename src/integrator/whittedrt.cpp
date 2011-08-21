/*
   FileName:      whittedrt.cpp

   Created Time:  2011-08-04 12:49:03

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "whittedrt.h"
#include "integratormethod.h"
#include "geometry/intersection.h"
#include "geometry/scene.h"
#include "bsdf/bsdf.h"
#include "light/light.h"

// radiance along a specific ray direction
Spectrum WhittedRT::Li( const Scene& scene , const Ray& r , const PixelSample& ps ) const
{
	if( r.m_Depth > 6 )
		return 0.0f;

	// get the intersection between the ray and the scene
	Intersection ip;
	if( false == scene.GetIntersect( r , &ip ) )
		return 0.0f;

	Spectrum t;

	// get bsdf
	Bsdf* bsdf = ip.primitive->GetMaterial()->GetBsdf( &ip );

	// lights
	Visibility visibility(scene);
	const vector<Light*>& lights = scene.GetLights();
	vector<Light*>::const_iterator it = lights.begin();
	while( it != lights.end() )
	{
		// only delta light is evaluated
		if( (*it)->IsDelta() )
		{
			Vector	lightDir;
			float	pdf;
			Spectrum ld = (*it)->sample_l( ip , 0 , lightDir , 0.1f , &pdf , visibility );
			if( ld.IsBlack() )
			{
				it++;
				continue;
			}
			Spectrum f = bsdf->f( -r.m_Dir , lightDir );
			if( f.IsBlack() )
			{
				it++;
				continue;
			}
			bool visible = visibility.IsVisible();
			if( visible )
				t += (ld * f * SatDot( lightDir , ip.normal ) / pdf);
		}
		it++;
	}

	// add reflection
	if( bsdf->NumComponents( BXDF_REFLECTION ) > 0 )
		t += SpecularReflection( scene , r , &ip , bsdf , this , ps );
	if( bsdf->NumComponents( BXDF_TRANSMISSION ) > 0 )
		t += SpecularRefraction( scene , r , &ip , bsdf , this , ps );

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
