/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include "sort.h"
#include "spectrum/spectrum.h"
#include "math/vector3.h"
#include "utility/enum.h"
#include "geometry/intersection.h"

class Bxdf;
class Intersection;
class BsdfSample;

#define	MAX_BXDF_COUNT 8

//! @brief BSDF implementation.
class	Bsdf
{
public:
	//! @brief Constructor taking intersection data.
    //! @param intersection     Intersection data of the point to be evaluted.
	Bsdf( const Intersection* intersection );

	//! @brief Get the number of components based on the type.
    //! @param type     The specific type to be checked.
    //! @return         The number of components in the type.
	unsigned NumComponents( BXDF_TYPE type = BXDF_ALL ) const;

	//! @brief Add a new bxdf in the BSDF, there will be at most 8 bxdf in it.
	//! @param The bxdf to be added.
	void AddBxdf( Bxdf* bxdf );

	//! @brief Evalute the value of BSDF based on the incoming and outgoing directions.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @param type The specific type to be considered.
    //! @return     The evaluted value of the BSDF.
	Spectrum f( const Vector& wo , const Vector& wi , BXDF_TYPE type = BXDF_ALL ) const;

    //! @brief Importance sampling for the bsdf.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @param type The specific bxdf type it considers during evaluation.
    //! @param bxdf_type The specific bxdf type which it selects among all bxdfs in the BSDF.
    //! @return     The evaluted BRDF value.
	Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf , BXDF_TYPE type = BXDF_ALL , BXDF_TYPE* bxdf_type = 0 ) const;

    //! @brief Evalute the pdf of an existance direction given the incoming direction.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @param type The specific bxdf type it considers during evaluation.
    //! @return     The probabilty of choosing the out-going direction based on the incoming direction.
	float Pdf( const Vector& wo , const Vector& wi , BXDF_TYPE type = BXDF_ALL ) const;

	//! @brief Get intersection information of the point at which the bsdf is evaluated.
    //! @return The intersection information of the point at which the bsdf is evaluated.
	const Intersection* GetIntersection() const { return &intersect; }

private:
    Bxdf*	m_bxdf[MAX_BXDF_COUNT] = {};    /**< List of Bxdf in the BSDF. */
	unsigned m_bxdfCount = 0;               /**< Number of Bxdf in the BSDF. */

    Vector nn;  /**< Normal at the point to be evaluted. */
    Vector sn;  /**< Bi-tangent at the point to be evaluated. */
    Vector tn;  /**< Tangent at the point to be evaluted. */

	// intersection for the bsdf
	const Intersection intersect;   /**<    Intersection information of the point to be evaluted. */

    //! @brief Transform a vector from world coordinate to shading coordinate.
    //! @param v    A vector in world coordiante.
    //! @return     Cooresponding vector in shading coordinate.
	Vector worldToLocal( const Vector& v ) const;
    
    //! @brief Transform a vector from shading coordinate to world coordinate.
    //! @param v    A vector in shading coordinate.
    //! @return     Cooresponding vector in world coordinate.
	Vector localToWorld( const Vector& v ) const;
};

// BSDF Inline Functions
inline float CosTheta(const Vector &w){
	return w.y; 
}

inline float AbsCosTheta(const Vector &w){
	return fabsf(w.y); 
}

inline float SinTheta2(const Vector &w){
    return max(0.f, 1.f - CosTheta(w)*CosTheta(w));
}

inline float CosDPhi( const Vector& w0 , const Vector& w1 ){
    return clamp( ( w0.x * w1.x + w0.z * w1.z ) / sqrt( (w0.x * w0.x + w0.z * w0.z)*(w1.x * w1.x + w1.z*w1.z) ) , -1.0f , 1.0f );
}
inline float SinTheta(const Vector &w){
    return sqrtf(SinTheta2(w));
}
inline float CosPhi(const Vector &w){
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

inline Vector SphericalVec( float theta , float phi ){
	float x = sin( theta ) * cos( phi );
	float y = cos( theta );
	float z = sin( theta ) * sin( phi );

	return Vector( x , y , z );
}

inline Vector SphericalVec( float sintheta , float costheta , float phi ){
	float x = sintheta * cos( phi );
	float y = costheta;
	float z = sintheta * sin( phi );
	return Vector( x , y , z );
}

inline bool SameHemiSphere( const Vector& wi , const Vector& wo ){
	return ( wi.y * wo.y ) > 0.0f;
}

