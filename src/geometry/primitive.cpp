/*
   FileName:      primitive.cpp

   Created Time:  2011-08-18 16:59:53

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#include "primitive.h"
#include "light/light.h"
#include "bbox.h"

// get emissive
Spectrum Primitive::GetEmissive( const Vector& wo , const Intersection& intersect ) const
{
	if( m_bEmissive == false )
		return 0.0f;

	Sort_Assert( m_mat->GetLight() != 0 );

	return m_mat->GetLight()->sample_l( intersect , wo ); 
}

// delete the cache
void Primitive::ClearBBoxCache()
{
	m_bbox.Delete();
}