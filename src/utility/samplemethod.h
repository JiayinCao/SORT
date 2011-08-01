/*
 * filename :	samplemethod.h
 *
 * programmer :	Cao Jiayin
 */

#ifndef	SORT_SAMPLEMETHOD
#define	SORT_SAMPLEMETHOD

#include "bsdf/bsdf.h"

/*
description :
	There are some basic sampling methods used for Monte Carlo ray tracing.
*/

// sampling a point on unit disk uniformly using Shirley's Mapping
// para 'u' : a canonical random variable
// para 'v' : a canonical random variable
// para 'x' : x position on the unit disk
// para 'y' : y position on the unit disk
inline void UniformSampleDisk( float u , float v , float& x , float& y )
{
	float r , theta;
	float su = 2.0f * u - 1.0f;
	float sv = 2.0f * v - 1.0f;
	float asu = fabs( su );
	float asv = fabs( sv );

	if( asu < asv )
	{
		r = asv;
		float factor = ( sv > 0.0f ) ? -1.0f : 1.0f;
		theta = factor * su / r + 4.0f + 2.0f * factor ;
	}else
	{
		r = asu;
		float factor = ( su > 0.0f ) ? 1.0f : -1.0f;
		theta = factor * sv / r - 2.0f * factor + 2.0f  ;
		if( theta < 0.0f )
			theta += 8.0f;
	}
	theta *= PI / 4.0f;

	x = cos( theta ) * r;
	y = sin( theta ) * r;
}

// sampling a vector in a hemisphere using cosine pdf
// para 'u' : a canonical random variable
// para 'v' : a canonical random variable
inline Vector CosSampleHemisphere( float u , float v )
{
	float x , y ;
	UniformSampleDisk( u , v , x , y );
	float z = sqrt( 1.0f - x * x - y * y );
	return Vector( x , y , z );
}

// cosine hemisphere pdf
inline float CosHemispherePdf( const Vector& v )
{
	return CosTheta(v) * INV_PI;
}

// sampling a vector in a hemisphere uniformly
// para 'u'	: a canonical random variable
// para 'v' : a canonical random variable
inline Vector UniformSampleHemisphere( float u , float v )
{
	float theta = acos( u );
	float phi = TWO_PI * v;

	return SphericalVec( theta , phi );
}

// uniformly sample hemisphere pdf
inline float UniformHemispherePdf( const Vector& v )
{
	return INV_TWOPI;
}

// sampling a vector in sphere uniformly
// para 'u'	: a canonical random variable
// para 'v'	: a canonical random variable
inline Vector UniformSampleSphere( float u , float v )
{
	float theta = acos( 1 - 2.0f * u );
	float phi = TWO_PI * v;
	return SphericalVec( theta , phi );
}

// pdf of uniformly sampling a vector on sphere
inline float UniformSpherePdf( const Vector& v )
{
	return INV_TWOPI * 0.5f;
}

#endif
