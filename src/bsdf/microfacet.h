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

#include "bxdf.h"
#include "bsdf.h"
#include "fresnel.h"

//! @brief Normal distribution function.
class MicroFacetDistribution
{
public:
	//! @brief Probabilty of facet with specific normal (h)
	virtual float D(const Vector& h) const = 0;
    
    //! @brief Visibility term of microfacet model, Smith shadow-masking function
    float G( const Vector& wo , const Vector& wi ) const {
        return G1( wo ) * G1( wi );
    }

	//! @brief Sampling a normal respect to the NDF.
    //! @param bs   Sample holind all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
	virtual Vector sample_f( const BsdfSample& bs , const Vector& wo ) const = 0;
    
    //! @brief PDF of sampling a specific normal direction
    //! @param wh   Normal direction to be sampled
    float Pdf( const Vector& wh ) const {
        return D( wh ) * AbsCosTheta(wh);
    }
    
protected:
    //! @brief Smith shadow-masking function G1
    virtual float G1( const Vector& v ) const  = 0;
};

//! @brief Blinn NDF.
class Blinn : public MicroFacetDistribution
{
public:
	//! @brief Constructor
    //! @param roughnessU    Roughness of the surface formed by the micro facets.
    //! @param roughnessV    Roughness of the surface formed by the micro facets.
	Blinn(float roughnessU, float roughnessV);
	
	//! @brief Probabilty of facet with specific normal (h)
	float D(const Vector& h) const override;
	
    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holind all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs , const Vector& wo ) const override;

private:
	float expU , expV , exp , expUV;      /**< Internal data used for NDF calculation. */
    float alphaU2 , alphaV2;
    
    //! @brief Smith shadow-masking function G1
    float G1( const Vector& v ) const override;
};

//! @brief Beckmann NDF.
class Beckmann : public MicroFacetDistribution
{
public:
    //! @brief Constructor
    //! @param roughnessU    Roughness of the surface formed by the micro facets.
    //! @param roughnessV    Roughness of the surface formed by the micro facets.
	Beckmann( float roughnessU , float roughnessV );

    //! @brief Probabilty of facet with specific normal (v)
    float D(const Vector& h) const override;
    
    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holind all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs , const Vector& wo ) const override;

private:
	float alphaU , alphaV;        /**< Internal data used for NDF calculation. */
    float alphaU2 , alphaV2 , alphaUV, alpha;
    
    //! @brief Smith shadow-masking function G1
    float G1( const Vector& v ) const override;
};

//! @brief GGX NDF.
class GGX : public MicroFacetDistribution
{
public:
    //! @brief Constructor
    //! @param roughness    Roughness of the surface formed by the micro facets.
	GGX( float roughnessU , float roughnessV );

    //! @brief Probabilty of facet with specific normal (h)
    float D(const Vector& h) const override;
    
    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holind all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs , const Vector& wo ) const override;

private:
	float alphaU , alphaV;        /**< Internal data used for NDF calculation. */
    float alphaU2 , alphaV2 , alphaUV , alpha;
    
    //! @brief Smith shadow-masking function G1
    float G1( const Vector& v ) const override;
};

//! @brief Interface for Microfacet bxdf.
class Microfacet : public Bxdf
{
protected:
	const MicroFacetDistribution* distribution = nullptr; /**< Normal distribution of micro facets. */
	const Fresnel* fresnel = nullptr;                     /**< Fresnel term. */

	//! @brief Get reflected direction based on incident direction and normal.
    //! @param v    Incoming direction.
    //! @param n    Normal of the surface.
    //! @return     Reflected direction.
	inline Vector	getReflected( Vector v , Vector n ) const;

	//! @brief Get refracted direction based on incident direction, normal and index of refraction.
    //! @param v        Incoming direction. It can come from either inside or outside of the surface.
    //! @param n        Surface normal.
    //! @param in_eta   Index of refraction inside the surface.
    //! @param ext_eta  Index of refraction outside the surface.
    //! @param inner_reflection     Whether it is a total innner relection.
    //! @return         Refracted vector based on Snell's law.
	inline Vector	getRefracted( Vector v , Vector n , float in_eta , float ext_eta , bool& inner_reflection ) const;
};

//! @brief Microfacet for reflection surfaces.
class MicroFacetReflection : public Microfacet
{
public:
	//! @brief Constructor
    //! @param reflectance      Direction hemisphere reflection.
    //! @param f                Fresnel term.
    //! @param d                NDF term.
    //! @param v                Visibility term.
	MicroFacetReflection(const Spectrum &reflectance, const Fresnel* f , const MicroFacetDistribution* d );
	
    //! @brief Evaluate the BRDF
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The evaluted BRDF value.
	Spectrum f( const Vector& wo , const Vector& wi ) const override;
	
    //! @brief Importance sampling for the microfacet brdf.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The evaluted BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;

    //! @brief Evalute the pdf of an existance direction given the incoming direction.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The probabilty of choosing the out-going direction based on the incoming direction.
	float Pdf( const Vector& wo , const Vector& wi ) const override;
    
private:
    Spectrum R;                   /**< Direction-hemisphere reflection. */
};

/////////////////////////////////////////////////////////////////////
// microfacet refraction bxdf
// Refer to "Microfacet Models for Refraction through Rough Surfaces" for further detail
class MicroFacetRefraction : public Microfacet
{
public:
    //! @brief Constructor
    //! @param transmittance    Direction hemisphere transmittance.
    //! @param f                Fresnel term.
    //! @param d                NDF term.
    //! @param v                Visibility term.
    //! @param etaI             Index of refraction of the side that normal points
    //! @param etaT             Index of refraction of the other side that normal points
	MicroFacetRefraction(const Spectrum &transmittance, const MicroFacetDistribution* d , float etaI , float etaT );
	
    //! @brief Evaluate the BRDF
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The evaluted BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
    
    //! @brief Importance sampling for the microfacet btdf.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The evaluted BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;
    
    //! @brief Evalute the pdf of an existance direction given the incoming direction.
    //! @param wo   Exitance direction in shading coordinate.
    //! @param wi   Incomiing direction in shading coordinate.
    //! @return     The probabilty of choosing the out-going direction based on the incoming direction.
    float Pdf( const Vector& wo , const Vector& wi ) const override;

private:
	float	            etaI;               /**< Index of refraction of the side that normal points. */
	float	            etaT;               /**< Index of refraction of the other side that normal points. */
    Spectrum            T;                  /**< Direction-hemisphere transmittance. */
    FresnelDielectric   fresnel;            /**< Dielectric fresnel in microfacet tranmittance model. */
};
