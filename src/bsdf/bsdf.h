/*
   FileName:      bsdf.h

   Created Time:  2011-08-04 12:52:18

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

#ifndef	SORT_BSDF
#define	SORT_BSDF

// include the header
#include "sort.h"
#include "spectrum/spectrum.h"
#include "geometry/vector.h"
#include "utility/enum.h"
#include <math.h>

// pre-declera class
class Bxdf;
class Intersection;

#define	MAX_BXDF_COUNT 8

////////////////////////////////////////////////////////////////////////
// definition of bsdf
class	Bsdf
{
// public method
public:
	// default constructor
	Bsdf( const Intersection* );
	// destructor
	~Bsdf();

	// get the number of bxdf
	unsigned NumComponents( BXDF_TYPE type = BXDF_ALL ) const;

	// add a new bxdf
	// para 'bxdf' : a bxdf to add
	void AddBxdf( Bxdf* bxdf );

	// evaluate bxdf
	// para 'wi' : input vector
	// para 'wo' : output vector
	Spectrum f( const Vector& wo , const Vector& wi , BXDF_TYPE type = BXDF_ALL ) const;

	// sample ray from bsdf
	// para 'wi' : input vector
	// para 'wo' : output vector
	Spectrum sample_f( const Vector& wo , Vector& wi , float* pdf , BXDF_TYPE type = BXDF_ALL ) const;

// private field
private:
	// the list for the bxdf
	Bxdf*	m_bxdf[MAX_BXDF_COUNT];

	// current bsdf
	unsigned m_bxdfCount;

	// the vectors
	Vector nn , sn , tn;

	// transform a vector from world to shading coordinate
	// para 'v' : a vector in world space
	// result   : a transformed vector in shading coordinate
	Vector _worldToLocal( const Vector& v ) const;
	// transform a vector from shading coordinate to world coordinate
	// para 'v' : a vector in shading coordinate
	// result   : a transformed vector in world coordinate
	Vector _localToWorld( const Vector& v ) const;
};

// BSDF Inline Functions
inline float CosTheta(const Vector &w) 
{ 
	return w.y; 
}
inline float AbsCosTheta(const Vector &w) 
{ 
	return fabsf(w.y); 
}
inline float SinTheta2(const Vector &w) 
{
    return max(0.f, 1.f - CosTheta(w)*CosTheta(w));
}

inline float SinTheta(const Vector &w) 
{
    return sqrtf(SinTheta2(w));
}

inline float CosPhi(const Vector &w) 
{
    float sintheta = SinTheta(w);
    if (sintheta == 0.f) return 1.f;
    return clamp(w.x / sintheta, -1.f, 1.f);
}

inline float SinPhi(const Vector &w) {
    float sintheta = SinTheta(w);
    if (sintheta == 0.f) return 0.f;
    return clamp(w.z / sintheta, -1.f, 1.f);
}

inline bool SameHemisphere(const Vector &w, const Vector &wp) {
    return w.y * wp.y > 0.f;
}

inline float SphericalTheta(const Vector &v) {
    return acosf(clamp(v.y, -1.f, 1.f));
}

inline float SphericalPhi(const Vector &v) {
    float p = atan2f(v.z, v.x);
    return (p < 0.f) ? p + 2.f*PI : p;
}

inline Vector SphericalVec( float theta , float phi )
{
	float x = sin( theta ) * cos( phi );
	float y = cos( theta );
	float z = sin( theta ) * sin( phi );

	return Vector( x , y , z );
}

inline bool SameHemiSphere( const Vector& wi , const Vector& wo )
{
	return ( wi.y * wo.y ) > 0.0f;
}

#endif
