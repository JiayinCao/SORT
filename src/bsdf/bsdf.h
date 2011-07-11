/*
 * filename :	bsdf.h
 *
 * programmer :	Cao Jiayin
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
	unsigned NumComponents() const;

	// add a new bxdf
	// para 'bxdf' : a bxdf to add
	void AddBxdf( Bxdf* bxdf );

	// evaluate bxdf
	// para 'wi' : input vector
	// para 'wo' : output vector
	Spectrum f( const Vector& wo , const Vector& wi , BXDF_TYPE type = BXDF_ALL ) const;

// private field
private:
	// the list for the bxdf
	Bxdf*	m_bxdf[MAX_BXDF_COUNT];

	// current bsdf
	unsigned m_bsdfCount;

	// the vectors
	Vector nn , sn , tn;

	// transform the vector from world to shading coordinate
	// para 'v' : a vector in world space
	// result   : a transformed vector in shading coordinate
	Vector _worldToLocal( const Vector& v ) const;
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

#endif
