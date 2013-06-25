/*
   FileName:      matte.cpp

   Created Time:  2011-08-04 12:46:36

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "matte.h"
#include "utility/define.h"
#include "bsdf/bsdf.h"
//#include "bsdf/lambert.h"
#include "managers/memmanager.h"
#include "bsdf/orennayar.h"

IMPLEMENT_CREATOR( Matte );

// constructor
Matte::Matte()
{
	_init();
}

// destructor
Matte::~Matte()
{
	// delete the texture
	SAFE_DELETE( m_d );
}

// initiailize default value and register properties
void Matte::_init()
{
	m_d = 0;
	m_scale = Spectrum( 1.0f , 1.0f , 1.0f );

	_registerAllProperty();
}

// get bsdf
Bsdf* Matte::GetBsdf( const Intersection* intersect ) const
{
	Spectrum color = m_scale;
	if( m_d )
		color *= m_d->Evaluate( intersect );

	Bsdf* bsdf = SORT_MALLOC(Bsdf)( intersect );
/*	Lambert* lambert = SORT_MALLOC(Lambert)();
	lambert->SetColor( color );
	bsdf->AddBxdf( lambert );
*/
	OrenNayar* orennayar = SORT_MALLOC(OrenNayar)( color , 20.0f/180.0f*3.14f );
	bsdf->AddBxdf( orennayar );
	
	return bsdf;
}

// register property
void Matte::_registerAllProperty()
{
	_registerProperty( "color" , new ColorProperty( this ) );
	_registerProperty( "scale" , new ScaleProperty( this ) );
}
