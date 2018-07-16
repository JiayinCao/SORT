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
#include "fresnel.h"

//! @brief Normal distribution function.
class MicroFacetDistribution
{
public:
	//! @brief Probabilty of facet with specific normal (h)
	virtual float D(const Vector& h) const = 0;

	//! @brief Sampling a normal respect to the NDF.
    //! @param bs   Sample holind all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
	virtual Vector sample_f( const BsdfSample& bs , const Vector& wo ) const = 0;
};

//! @brief Blinn NDF.
class Blinn : public MicroFacetDistribution
{
public:
	//! @brief Constructor
    //! @param roughness    Roughness of the surface formed by the micro facets.
	Blinn( float roughness );
	
	//! @brief Probabilty of facet with specific normal (h)
	float D(const Vector& h) const override;
	
    //! @brief Sampling a normal respect to the NDF.
    //!
    //! Refer to this
    //! <a href="https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/">blog</a>
    //! for detail explanation of the implementation.
    //! @param bs   Sample holind all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs , const Vector& wo ) const override;

private:
	float exp;      /**< Internal data used for NDF calculationg. */
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
	float alphaU , alphaV;        /**< Internal data used for NDF calculationg. */
    float alphaU2 , alphaV2 , alphaUV;
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
    //! Refer to this
    //! <a href="https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/">blog</a>
    //! for detail explanation of the implementation.
    //! @param bs   Sample holind all necessary random variables.
    //! @param wo   Outgoing direction
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs , const Vector& wo ) const override;

private:
	float alphaU , alphaV;        /**< Internal data used for NDF calculationg. */
    float alphaU2 , alphaV2 , alphaUV;
};

//! @brief Visibility term.
class VisTerm
{
public:
    //! @brief Evalute visibility term.
    //! @param NoL  Cosine value of the angle between light and normal.
    //! @param NoV  Cosine value of the angle between view direction and normal
    //! @param VoH  Cosine value of the angle between view and middle vector
    //! @param NoH  Cosine value of the angle between normal and middle vector
    //! @return     Visibility term
	virtual float Vis_Term( float NoL , float NoV , float VoH , float NoH ) = 0;
};

//! @brief Implicit visibility term.
class VisImplicit : public VisTerm
{
public:
    float Vis_Term( float NoL , float NoV , float VoH , float NoH) override;
};

//! @brief Neumann visibility term.
class VisNeumann : public VisTerm
{
public:
	float Vis_Term( float NoL , float NoV , float VoH , float NoH) override;
};

//! @brief Kelemen visibility term.
class VisKelemen : public VisTerm
{
public:
    float Vis_Term( float NoL , float NoV , float VoH , float NoH) override;
};

//! @brief Schlick visibility term.
class VisSchlick : public VisTerm
{
public:
    //! @brief Constructor
    //! @param rough    Roughness value.
	VisSchlick( float rough ): roughness(rough) {}
    
    float Vis_Term( float NoL , float NoV , float VoH , float NoH) override;

private:
	float roughness;    /**< Roughness value */
};

//! @brief Smith visibility term.
class VisSmith : public VisTerm
{
public:
    //! @brief Constructor
    //! @param rough    Roughness value.
	VisSmith( float rough ): roughness(rough) {}
    
    float Vis_Term( float NoL , float NoV , float VoH , float NoH) override;

private:
	float roughness;    /**< Roughness value */
};

//! @brief Smith Joint Approximation visibility term.
class VisSmithJointApprox : public VisTerm
{
public:
    //! @brief Constructor
    //! @param rough    Roughness value.
	VisSmithJointApprox( float rough ): roughness(rough) {}
    
    float Vis_Term( float NoL , float NoV , float VoH , float NoH) override;

private:
	float roughness;    /**< Roughness value */
};

//! @brief CookTorrance visibility term.
class VisCookTorrance : public VisTerm
{
public:
    float Vis_Term( float NoL , float NoV , float VoH , float NoH) override;
};

//! @brief Interface for Microfacet bxdf.
class Microfacet : public Bxdf
{
protected:
	Spectrum R;                                     /**< Direction-hemisphere reflection. */
	MicroFacetDistribution* distribution = nullptr; /**< Normal distribution of micro facets. */
	Fresnel* fresnel = nullptr;                     /**< Fresnel term. */
	VisTerm* visterm = nullptr;                     /**< Visibility term. */

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
	MicroFacetReflection(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v );
	
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
};

/////////////////////////////////////////////////////////////////////
// microfacet refraction bxdf
// Refer to "Microfacet Models for Refraction through Rough Surfaces" for further detail
class MicroFacetRefraction : public Microfacet
{
public:
    //! @brief Constructor
    //! @param reflectance      Direction hemisphere reflection.
    //! @param f                Fresnel term.
    //! @param d                NDF term.
    //! @param v                Visibility term.
    //! @param ieta             Index of refraction inside the surface.
    //! @param eeta             Index of refraction outside the surface.
	MicroFacetRefraction(const Spectrum &reflectance, Fresnel* f , MicroFacetDistribution* d , VisTerm* v , float ieta , float eeta );
	
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
	float	eta_in;     /**< Index of refraction inside the surface. */
	float	eta_ext;    /**< Index of refraction outside the surface. */
};
