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
#include "sampler/sample.h"
#include "utility/sassert.h"

// constructor
Bsdf::Bsdf( const Intersection* _intersect ) : intersect( *_intersect )
{
	nn = intersect.normal;
	tn = Normalize(Cross( nn , intersect.tangent ));
	sn = Cross( tn , nn );
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
	if( m_bxdfCount == MAX_BXDF_COUNT || bxdf == 0 || bxdf->m_weight.IsBlack() )
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
			r += m_bxdf[i]->f( swo , swi ) * m_bxdf[i]->m_weight;
	}

	return r;
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

// sample a ray from bsdf
Spectrum Bsdf::sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf , BXDF_TYPE type , BXDF_TYPE* bxdf_type ) const
{
	unsigned com_num = NumComponents(type);
	if( com_num == 0 )
	{
		if( pdf ) *pdf = 0.0f;
		if( bxdf_type ) *bxdf_type = BXDF_NONE;
		return 0.0f;
	}
	int bsdf_id = min( (int)(bs.t*(float)com_num) , (int)(com_num-1) );
	Bxdf* bxdf = 0;
	int count = bsdf_id;
	for( unsigned i = 0 ; i < m_bxdfCount ; ++i )
		if( m_bxdf[i]->MatchFlag(type) ){
			if( count == 0 )
			{
				bxdf = m_bxdf[i];
				break;
			}
			count--;
		}

	Sort_Assert( bxdf != 0 );

	// transform the 'wo' from world space to shading coordinate
	Vector swo = Normalize(_worldToLocal( wo ));

	// sample the direction
	Spectrum t = bxdf->sample_f( swo , wi , bs , pdf ) * bxdf->m_weight;

	// if there is no properbility of sampling that direction , just return 0.0f
	if( pdf && *pdf == 0.0f ) return 0.0f;
	if( bxdf_type ) *bxdf_type = bxdf->GetType();

	// setup pdf
	if( pdf  && ( com_num > 1 ) )
	{
		for( unsigned i = 0; i < m_bxdfCount ; ++i )
			if( m_bxdf[i] != bxdf && m_bxdf[i]->MatchFlag( type ) )
				*pdf += m_bxdf[i]->Pdf( wo , wi );
	}
	if( pdf ) *pdf /= com_num;
	
	for( unsigned i = 0 ; i < m_bxdfCount ; ++i )
		if( bxdf != m_bxdf[i] && m_bxdf[i]->MatchFlag(type) )
			t += m_bxdf[i]->f(wo,wi) * m_bxdf[i]->m_weight;
	
	// transform the direction back
	wi = _localToWorld( wi );

	return t;
}

// get the pdf according to sampled direction
float Bsdf::Pdf( const Vector& wo , const Vector& wi , BXDF_TYPE type ) const
{
	Vector lwo = _worldToLocal( wo );
	Vector lwi = _worldToLocal( wi );

	float pdf = 0.0f;
	for( unsigned i = 0 ; i < m_bxdfCount ; ++i )
		if( m_bxdf[i]->MatchFlag( type ) )
			pdf += m_bxdf[i]->Pdf( lwo , lwi );
	return pdf;
}
