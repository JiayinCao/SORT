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
class BsdfSample;

#define	MAX_BXDF_COUNT 8

//! @brief BSDF implementation.
class	Bsdf
{
public:
	//! @brief Constructor taking intersection data.
    //! @param intersection     Intersection data of the point to be Evaluated.
    //! @param sub_bsdf         Sub-bsdf doesn't transform the light again.
	Bsdf( const Intersection* intersection , bool sub_bsdf = false );

	//! @brief Get the number of components based on the type.
    //! @param type     The specific type to be checked.
    //! @return         The number of components in the type.
	unsigned NumComponents( BXDF_TYPE type = BXDF_ALL ) const;

	//! @brief Add a new bxdf in the BSDF, there will be at most 8 bxdf in it.
	//! @param The bxdf to be added.
	void AddBxdf( const Bxdf* bxdf );

	//! @brief Evaluate the value of BSDF based on the Incident and outgoing directions.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param type The specific type to be considered.
    //! @return     The Evaluated value of the BSDF.
	Spectrum f( const Vector& wo , const Vector& wi , BXDF_TYPE type = BXDF_ALL ) const;

    //! @brief Importance sampling for the bsdf.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @param type The specific bxdf type it considers during evaluation.
    //! @param bxdf_type The specific bxdf type which it selects among all bxdfs in the BSDF.
    //! @return     The Evaluated BRDF value.
	Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf , BXDF_TYPE type = BXDF_ALL , BXDF_TYPE* bxdf_type = 0 ) const;

    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param type The specific bxdf type it considers during evaluation.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
	float Pdf( const Vector& wo , const Vector& wi , BXDF_TYPE type = BXDF_ALL ) const;

	//! @brief Get intersection information of the point at which the bsdf is evaluated.
    //! @return The intersection information of the point at which the bsdf is evaluated.
	const Intersection* GetIntersection() const { return &intersect; }

private:
    const Bxdf*	m_bxdf[MAX_BXDF_COUNT] = {};    /**< List of Bxdf in the BSDF. */
	unsigned m_bxdfCount = 0;                   /**< Number of Bxdf in the BSDF. */

    Vector nn;  /**< Normal at the point to be Evaluated. */
    Vector sn;  /**< Bi-tangent at the point to be evaluated. */
    Vector tn;  /**< Tangent at the point to be Evaluated. */
    
    const bool  m_SubBSDF;      /**< SubBSDF doesn't transform light direction again. */

	// intersection for the bsdf
    const Intersection intersect;   /**<    Intersection information of the point to be Evaluated. */

    //! @brief Transform a vector from world coordinate to shading coordinate.
    //! @param v        A vector in world coordinate.
    //! @param force    Force transform regardless whether this bsdf is sub-bsdf or not.
    //! @return         corresponding vector in shading coordinate.
	Vector worldToLocal( const Vector& v , bool force = false ) const;
    
    //! @brief Transform a vector from shading coordinate to world coordinate.
    //! @param v    A vector in shading coordinate.
    //! @return     corresponding vector in world coordinate.
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
    return std::max(0.f, 1.f - CosTheta(w)*CosTheta(w));
}

inline float CosTheta2(const Vector &w){
    return CosTheta(w) * CosTheta(w);
}

inline float TanTheta2(const Vector &w){
    return 1.0f / CosTheta2(w) - 1.0f;
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

inline float SinPhi2(const Vector &w) {
    const float sinPhi = SinPhi(w);
    return sinPhi * sinPhi;
}

inline float CosPhi2(const Vector &w) {
    const float cosPhi = CosPhi(w);
    return cosPhi * cosPhi;
}

inline float TanTheta( const Vector& w){
    return SinTheta(w) / CosTheta(w);
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

inline Vector reflect(const Vector& v, const Vector& n){
    return (2.0f * Dot(v, n)) * n - v;
}

// an optimized version only works in shading coordinate
inline Vector reflect(const Vector& v) {
    return Vector(-v.x, v.y, -v.z);
}

//! @param v                    Incident direction. It can come from either inside or outside of the surface.
//! @param n                    Surface normal.
//! @param in_eta               Index of refraction inside the surface.
//! @param ext_eta              Index of refraction outside the surface.
//! @param inner_reflection     Whether it is a total inner reflection.
//! @return                     Refracted vector based on Snell's law.
//! @Note                       Both vectors ( the first parameter and the returned value ) should be pointing outside the surface.
inline Vector refract(const Vector& v, const Vector& n, float in_eta, float ext_eta, bool& inner_reflection){
    const float coso = Dot(v, n);
    const float eta = coso > 0 ? (ext_eta / in_eta) : (in_eta / ext_eta);
    const float t = 1.0f - eta * eta * std::max(0.0f, 1.0f - coso * coso);

    // total inner reflection
    inner_reflection = (t <= 0.0f);
    if (inner_reflection)
        return Vector(0.0f, 0.0f, 0.0f);
    const float scale = coso < 0.0f ? -1.0f : 1.0f;
    return -eta * v + (eta * coso - scale * sqrt(t)) * n;
}
