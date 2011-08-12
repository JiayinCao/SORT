/*
   FileName:      samplemethod.h

   Created Time:  2011-08-04 12:43:30

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_SAMPLEMETHOD
#define	SORT_SAMPLEMETHOD

#include <algorithm>
#include "bsdf/bsdf.h"
#include "managers/logmanager.h"
#include "texture/texture.h"
#include <vector>

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
	}else if( asv > asu )
	{
		r = asu;
		float factor = ( su > 0.0f ) ? 1.0f : -1.0f;
		theta = factor * sv / r - 2.0f * factor + 2.0f  ;
		if( theta < 0.0f )
			theta += 8.0f;
	}else
	{
		x = 0.0f;
		y = 0.0f;
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

// one dimensional distribution
class Distribution1D
{
// public method
public:
	// constructor
	Distribution1D( const float* f , unsigned n ):
		count(n)
	{
		cdf = 0;
		sum = 0.0f;
		if( f == 0 || n == 0 )
			return;

		cdf = new float[n+1];
		cdf[0] = 0;
		for( unsigned i = 0 ; i < n ; i++ )
			cdf[i+1] = cdf[i] + f[i];
		sum = cdf[n];
		for( unsigned i = 0 ; i < n+1 ; i++ )
			cdf[i] /= sum;
	}
	// destructor
	~Distribution1D(){ SAFE_DELETE_ARRAY(cdf); }

	// get a discrete sample
	// para 'u' : a canonical random variable
	// para 'pdf' : propery density function value for the sample
	// result   : corresponding bucket straddle the u, -1 if there is no data in the distribution
	int SampleDiscrete( float u , float* pdf ) const
	{
		if( count == 0 || cdf == 0 )
			return -1;
		if( u >= 1.0f || u < 0.0f )
			LOG_ERROR<<"Invalid canonical random variable in Distribution1D sample method"<<CRASH;

		float* target = lower_bound( cdf , cdf + count + 1 , u );
		int offset = (u<=0.0f)? 0:(int)(target-cdf-1);
		if( pdf ) 
			*pdf = cdf[offset+1] - cdf[offset];
		return offset;
	}

	// get a contineous sample
	// para 'u' : a canonical random variable
	// para 'pdf' : property density function value for the sample
	float SampleContinuous( float u , float* pdf ) const
	{
		if( count == 0 || cdf == 0 )
			return -1.0f;
		if( u >= 1.0f || u < 0.0f )
			LOG_ERROR<<"Invalid canonical random variable in Distribution1D sample method"<<CRASH;

		float* target = lower_bound( cdf , cdf+count+1 , u );
		int offset = (u<=0.0f)?0:(int)(target-cdf-1);
		if( pdf )
			*pdf = (cdf[offset+1]-cdf[offset])*count;
		float du = ( u - cdf[offset] ) / ( cdf[offset+1] - cdf[offset] );
		return ( du + (float)offset ) / (float)count;
	}

	// get the sum of the original data
	float GetSum() const
	{ return sum; }

	// get the count
	unsigned GetCount() const
	{ return count; }

// private field
private:
	const unsigned	count;
	float*	cdf;
	float	sum;
};

// two dimensional distribution
class Distribution2D
{
// public method
public:
	// default constructor
	Distribution2D( const float* data , unsigned nu , unsigned nv )
	{_init( data , nu , nv );}
	Distribution2D( const Texture* tex )
	{
		unsigned nu = tex->GetWidth();
		unsigned nv = tex->GetHeight();
		if( nu == 0 || nv == 0 )
			LOG_ERROR<<"One of the dimensions for the texture is zero!"<<CRASH;
		float* data = new float[nu*nv];
		for( unsigned i = 0 ; i < nv ; i++ )
		{
			unsigned offset = i * nu;
			for( unsigned j = 0 ; j < nu ; j++ )
				data[offset+j] = tex->GetColor( j , i ).GetIntensity();
		}
		_init( data , nu , nv );
		SAFE_DELETE_ARRAY(data);
	}
	// destructor
	~Distribution2D()
	{
		SAFE_DELETE( marginal );
		for( unsigned i = 0 ; i < m_nv ; i++ )
			SAFE_DELETE(pConditions[i]);
		pConditions.clear();
	}

	// get a sample point
	void SampleContinuous( float u , float v , float uv[2] , float* pdf )
	{
		float pdf0 , pdf1;
		uv[1] = marginal->SampleContinuous( v , &pdf1 );
		int vi = (int)(uv[1] * m_nv);
		if( vi > (int)(m_nv - 1) )
			vi = (int)(m_nv - 1);
		uv[0] = pConditions[vi]->SampleContinuous( u , &pdf0 );

		if( pdf )
			*pdf = pdf0 * pdf1;
	}

// private field
private:
	// the distribution in each row
	std::vector<Distribution1D*>	pConditions;
	// the marginal sampleing distribution
	Distribution1D*			marginal;
	// the size for the two dimensions
	unsigned m_nu , m_nv;

	// initialize data
	void _init( const float* data , unsigned nu , unsigned nv )
	{
		for( unsigned i = 0 ; i < nv ; i++ )
			pConditions.push_back( new Distribution1D( &data[i*nu] , nu ) );
		float* m = new float[nv];
		for( unsigned i = 0 ; i < nv ; i++ )
			m[i] = pConditions[i]->GetSum();
		marginal = new Distribution1D( m , nv );
		m_nu = nu;
		m_nv = nv;
		delete[] m;
	}
};
#endif
