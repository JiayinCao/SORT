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
#include "bsdf/lambert.h"
#include "managers/memmanager.h"

IMPLEMENT_CREATOR( Matte );

// constructor
Matte::Matte()
{
	m_color = Spectrum( 0.1f , 0.1f , 0.1f );
}

// get bsdf
Bsdf* Matte::GetBsdf( const Intersection* intersect ) const
{
	Bsdf* bsdf = SORT_MALLOC(Bsdf)( intersect );
	Lambert* lambert = SORT_MALLOC(Lambert)( m_color );
	bsdf->AddBxdf( lambert );
	lambert->m_weight = 1.0f;
	return bsdf;
}