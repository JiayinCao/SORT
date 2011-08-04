/*
   FileName:      bsdf.cpp

   Created Time:  2011-08-04 12:52:15

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "bsdf.h"
#include "bxdf.h"
#include "geometry/intersection.h"

// constructor
Bsdf::Bsdf( const Intersection* intersect )
{
	m_bxdfCount = 0;

	nn = intersect->normal;
	tn = Normalize(Cross( nn , intersect->tangent ));
	sn = Cross( tn , nn );
}

// destructor
Bsdf::~Bsdf()
{
}

// get the number of components in current bsdf
unsigned Bsdf::NumComponents( BXDF_TYPE type ) const
{
	unsigned count = 0;
	for( unsigned i = 0 ; i < m_bxdfCount ; i++ )
	{
		if( m_bxdf[i]->MatchFlag( type ) )
			count++;
	}
	return count;
}

// add a new bxdf
void Bsdf::AddBxdf( Bxdf* bxdf )
{
	if( m_bxdfCount == MAX_BXDF_COUNT )
		return;
	m_bxdf[m_bxdfCount] = bxdf ;
	m_bxdfCount++;
}

// evaluate bxdf
Spectrum Bsdf::f( const Vector& wo , const Vector& wi , BXDF_TYPE type ) const
{
	// the result
	Spectrum r;

	Vector swo = _worldToLocal( wo );
	Vector swi = _worldToLocal( wi );

	for( unsigned i = 0 ; i < m_bxdfCount ; i++ )
	{
		if( m_bxdf[i]->MatchFlag( type ) )
			r += m_bxdf[i]->f( swo , swi );
	}

	return r;
}

// sample a ray from bsdf
Spectrum Bsdf::sample_f( const Vector& wo , Vector& wi , float* pdf , BXDF_TYPE type ) const
{
	Vector swo = Normalize(_worldToLocal( wo ));
	Vector swi;

	for( unsigned i = 0 ; i < m_bxdfCount ; i++ )
	{
		if( m_bxdf[i]->MatchFlag( type ) )
		{
			Spectrum t = m_bxdf[i]->sample_f( swo , swi , pdf );
			wi = _localToWorld( swi );
			return t;
		}
	}

	return Spectrum();
}

// transform vector from world coordinate to shading coordinate
Vector Bsdf::_worldToLocal( const Vector& v ) const
{
	return Vector( Dot(v,sn) , Dot(v,nn) , Dot(v,tn) );
}

// transform vector from shading coordinate to world coordinate
Vector Bsdf::_localToWorld( const Vector& v ) const
{
	return Vector( 	v.x * sn.x + v.y * nn.x + v.z * tn.x ,
					v.x * sn.y + v.y * nn.y + v.z * tn.y ,
					v.x * sn.z + v.y * nn.z + v.z * tn.z );
}
