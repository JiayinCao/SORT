/*
   FileName:      normaltexture.cpp

   Created Time:  2011-08-04 12:45:22

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header
#include "normaltexture.h"
#include "managers/logmanager.h"
#include "geometry/intersection.h"

IMPLEMENT_CREATOR( NormalTexture );

// get the texture value
Spectrum NormalTexture::GetColor( int x , int y ) const
{
	return Spectrum();
}

// get the texture value
Spectrum NormalTexture::Evaluate( const Intersection* intersect ) const
{
	if( intersect )
		return Spectrum( intersect->normal.x , intersect->normal.y , intersect->normal.z );

	return Spectrum();
}

// whether the image supports outputing
bool NormalTexture::CanOutput() const
{
	LOG_WARNING<<"NormalTexture doesn't support outputing."<<ENDL;

	return false;
}
