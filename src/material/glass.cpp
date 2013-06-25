/*
   FileName:      glass.cpp

   Created Time:  2011-08-04 12:46:24

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include headers
#include "glass.h"
#include "managers/memmanager.h"
#include "bsdf/bsdf.h"
#include "bsdf/reflection.h"
#include "bsdf/refraction.h"

IMPLEMENT_CREATOR( Glass );

// default constructor
Glass::Glass()
{
	eta_i = 1.0f;
	eta_t = 1.1f;

	_registerAllProperty();
}

// register all properties
void Glass::_registerAllProperty()
{
	_registerProperty( "eta_i" , new EtaIProperty( this ) );
	_registerProperty( "eta_t" , new EtaTProperty( this ) );
	_registerProperty( "color" , new ColorProperty( this ) );
}

// get bsdf
Bsdf* Glass::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC(Bsdf)(intersect);

	Fresnel* fresnel = SORT_MALLOC(FresnelDielectric)( eta_i , eta_t );

	bsdf->AddBxdf( SORT_MALLOC(Reflection)(fresnel , color ) );
	bsdf->AddBxdf( SORT_MALLOC(Refraction)(eta_i,eta_t,fresnel , color ) );

	return bsdf;	
}

