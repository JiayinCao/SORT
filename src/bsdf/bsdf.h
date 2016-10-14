/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2016 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef	SORT_BSDF
#define	SORT_BSDF

// include the header
#include "sort.h"
#include "spectrum/spectrum.h"
#include "math/vector3.h"
#include "utility/enum.h"
#include <math.h>
#include "geometry/intersection.h"

// pre-declera class
class Bxdf;
class Intersection;
class BsdfSample;

#define	MAX_BXDF_COUNT 8

////////////////////////////////////////////////////////////////////////
// definition of bsdf
class	Bsdf
{
// public method
public:
	// default constructor
	Bsdf( const Intersection* );

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
	Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf , BXDF_TYPE type = BXDF_ALL , BXDF_TYPE* bxdf_type = 0 ) const;

	// get pdf according to the sampled method
	// para 'wo' : out going direction
	// para 'wi' : incoming direction
	float Pdf( const Vector& wo , const Vector& wi , BXDF_TYPE type = BXDF_ALL ) const;

	// Get current intersection
	const Intersection* GetIntersection() const { return &intersect; }

// private field
private:
	// the list for the bxdf
    Bxdf*	m_bxdf[MAX_BXDF_COUNT] = {};

	// current bsdf
	unsigned m_bxdfCount = 0;

	// the vectors
	Vector nn , sn , tn;

	// intersection for the bsdf
	const Intersection intersect;

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

inline Vector SphericalVec( float sintheta , float costheta , float phi )
{
	float x = sintheta * cos( phi );
	float y = costheta;
	float z = sintheta * sin( phi );

	return Vector( x , y , z );
}

inline bool SameHemiSphere( const Vector& wi , const Vector& wo )
{
	return ( wi.y * wo.y ) > 0.0f;
}

#endif
