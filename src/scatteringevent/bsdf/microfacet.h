/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2022 by Jiayin Cao - All rights reserved.

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
#include "spectrum/spectrum.h"
#include "scatteringevent/bsdf/bxdf_utils.h"
#include "multi_scattering_lut.h"

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMirror, "mirror")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMirror, Tsl_float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMirror, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMirror)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacetReflectionGGX, "microfacet_reflection_ggx")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionGGX, Tsl_float3, eta)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionGGX, Tsl_float3, absorption)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionGGX, Tsl_float,  roughness_u)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionGGX, Tsl_float,  roughness_v)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionGGX, Tsl_float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionGGX, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacetReflectionGGX)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacetReflectionBlinn, "microfacet_reflection_blinn")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBlinn, Tsl_float3, eta)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBlinn, Tsl_float3, absorption)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBlinn, Tsl_float, roughness_u)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBlinn, Tsl_float, roughness_v)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBlinn, Tsl_float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBlinn, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacetReflectionBlinn)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacetReflectionBeckmann, "microfacet_reflection_beckmann")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBeckmann, Tsl_float3, eta)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBeckmann, Tsl_float3, absorption)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBeckmann, Tsl_float, roughness_u)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBeckmann, Tsl_float, roughness_v)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBeckmann, Tsl_float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionBeckmann, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacetReflectionBeckmann)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacetReflectionDielectric, "microfacet_dielectric")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionDielectric, Tsl_float, iorI)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionDielectric, Tsl_float, iorT)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionDielectric, Tsl_float, roughness_u)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionDielectric, Tsl_float, roughness_v)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionDielectric, Tsl_float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetReflectionDielectric, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacetReflectionDielectric)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacetRefractionGGX, "microfacet_refraction_ggx")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionGGX, Tsl_float, etaI)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionGGX, Tsl_float, etaT)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionGGX, Tsl_float, roughness_u)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionGGX, Tsl_float, roughness_v)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionGGX, Tsl_float3, transmittance)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionGGX, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacetRefractionGGX)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacetRefractionBlinn, "microfacet_refraction_blinn")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBlinn, Tsl_float, etaI)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBlinn, Tsl_float, etaT)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBlinn, Tsl_float, roughness_u)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBlinn, Tsl_float, roughness_v)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBlinn, Tsl_float3, transmittance)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBlinn, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacetRefractionBlinn)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacetRefractionBeckmann, "microfacet_refraction_beckmann")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBeckmann, Tsl_float, etaI)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBeckmann, Tsl_float, etaT)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBeckmann, Tsl_float, roughness_u)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBeckmann, Tsl_float, roughness_v)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBeckmann, Tsl_float3, transmittance)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacetRefractionBeckmann, Tsl_float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacetRefractionBeckmann)

//! @brief Normal distribution function.
class MicroFacetDistribution{
public:
    //! @brief constructor taking in roughness
    MicroFacetDistribution(float roughness):roughness(roughness){}

    //! @brief probability of facet with specific normal (h)
    virtual float D(const Vector& h) const = 0;

    //! @brief Visibility term of microfacet model, Smith shadow-masking function
    float G( const Vector& wo , const Vector& wi ) const {
        return G1( wo ) * G1( wi );
    }

    //! @brief Hemispherical-directional reflectance
    //!
    //! This function returns the total amount of engery reflected along one direction
    //! with the ambient light that has a value of 1.0.
    //! Mathematically speaking, it is
    //! E(\mu) = \int_{0}^{2*\pi} \int_{0}^{1} f(\mu_o, \mu_i, \phi) \mu_i d\mu_i d\phi
    //!
    //! @param wo   Outgoing direction.
    //! @return     Hemispherical-directional reflectance.
    float E(const Vector& wo) const;

    //! @brief Hemispherical-hemispherical reflectance
    //!
    //! This function returns the total amount of engery reflected along all directions.
    //! Mathematically speaking, it is
    //! Eavg() = 2.0 \int_{0}^{1} E(\mu) \mu d \mu
    //! This is actually exactly the same with the last equation on page 430, pbrt 3rd.
    //!
    //! @return     Hemispherical-hemispherical reflectance
    float Eavg() const;

    //! @brief Sampling a normal respect to the NDF.
    //! @param bs   Sample holding all necessary random variables.
    //! @return     Sampled normal direction based on the NDF.
    virtual Vector sample_f( const BsdfSample& bs ) const = 0;

    //! @brief PDF of sampling a specific normal direction
    //! @param wh   Normal direction to be sampled
    float Pdf( const Vector& wh ) const {
        return D( wh ) * absCosTheta(wh);
    }

    //! @brief Get the roughness of the distribution
    float Roughness() const {
        return roughness;
    }

protected:
    //! @brief  Roughness of the distribution, this is merely for multi-scattering brdf
    const float roughness;

    //! @brief Smith shadow-masking function G1
    virtual float G1( const Vector& v ) const  = 0;

    //! @brief Check if the two vectors are in the same hemisphere in shading coordinate
    bool SameHemiSphere(const Vector& wo, const Vector& wi) const { return wo.y * wi.y > 0.0f; }

    //! @brief Get the pointer to the pre-baked lut for integral
    virtual const float* GetE_Lut() const = 0;
    virtual const float* GetEAvg_Lut() const = 0;
};

//! @brief Blinn NDF.
class Blinn : public MicroFacetDistribution{
public:
    //! @brief Constructor
    //! @param roughnessU    Roughness of the surface formed by the micro facets.
    //! @param roughnessV    Roughness of the surface formed by the micro facets.
    Blinn(float roughnessU, float roughnessV);

    //! @brief probability of facet with specific normal (h)
    float D(const Vector& h) const override;

    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holding all necessary random variables.
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs ) const override;

private:
    float expU , expV , exp , expUV;      /**< Internal data used for NDF calculation. */
    float alphaU2 , alphaV2;

    //! @brief Smith shadow-masking function G1
    float G1( const Vector& v ) const override;

    //! @brief Get the pointer to the pre-baked lut for integral
    const float* GetE_Lut() const override { return nullptr; }
    const float* GetEAvg_Lut() const override { return nullptr; };
};

//! @brief Beckmann NDF.
class Beckmann : public MicroFacetDistribution{
public:
    //! @brief Constructor
    //! @param roughnessU    Roughness of the surface formed by the micro facets.
    //! @param roughnessV    Roughness of the surface formed by the micro facets.
    Beckmann( float roughnessU , float roughnessV );

    //! @brief probability of facet with specific normal (v)
    float D(const Vector& h) const override;

    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holding all necessary random variables.
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs ) const override;

private:
    float alphaU , alphaV;        /**< Internal data used for NDF calculation. */
    float alphaU2 , alphaV2 , alphaUV, alpha;

    //! @brief Smith shadow-masking function G1
    float G1( const Vector& v ) const override;

    //! @brief Get the pointer to the pre-baked lut for integral
    const float* GetE_Lut() const override { return nullptr; }
    const float* GetEAvg_Lut() const override { return nullptr; };
};

//! @brief GGX NDF.
class GGX : public MicroFacetDistribution{
public:
    //! @brief Constructor
    //! @param roughnessU    Roughness of the surface formed by the micro facets.
    //! @param roughnessV    Roughness of the surface formed by the micro facets.
    GGX( float roughnessU , float roughnessV );

    //! @brief probability of facet with specific normal (h)
    float D(const Vector& h) const override;

    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holding all necessary random variables.
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f( const BsdfSample& bs ) const override;

protected:
    float alphaU , alphaV;        /**< Internal data used for NDF calculation. */
    float alphaU2 , alphaV2 , alphaUV , alpha;

    //! @brief Smith shadow-masking function G1
    float G1( const Vector& v ) const override;

    //! @brief Get the pointer to the pre-baked lut for integral
    const float* GetE_Lut() const override { return multi_scattering_ggs_no_fresnel::g_ms_E; }
    const float* GetEAvg_Lut() const override { return multi_scattering_ggs_no_fresnel::g_ms_Eavg; };
};

enum MF_Dist_Type {
    MF_DIST_GGX = 0,
    MF_DIST_BLINN,
    MF_DIST_BECKMANN,
};

//! @brief Interface for Microfacet bxdf.
class Microfacet : public Bxdf{
public:
    //! @brief  Constructor
    //! @brief Constructor
    //! @param distType     Distribution type, could be "GGX", "Beckmann" , "Blinn".
    //! @param ru           Roughness along U axis.
    //! @param rv           Roughness along V axis.
    //! @param w            Weight of the bxdf
    //! @param t            Type of the bxdf
    Microfacet(RenderContext& rc, const MF_Dist_Type distType, float ru , float rv , const Spectrum& w, const BXDF_TYPE t , const Vector& n , bool doubleSided );

    //! @brief Constructor
    //! @param d        Normal distribution term
    //! @param w        Weight of the bxdf
    //! @param t        Type of the bxdf
    Microfacet(RenderContext& rc, const MicroFacetDistribution* d, const Spectrum& w, const BXDF_TYPE t , const Vector& n , bool doubleSided) : Bxdf(rc, w, t, n, doubleSided) , distribution(d) {}

protected:
    const MicroFacetDistribution* distribution = nullptr; /**< Normal distribution of micro facets. */
};

//! @brief Microfacet Reflection BRDF.
class MicroFacetReflection : public Microfacet{
public:
    //! @brief Constructor for constructing a mirror bxdf.
    //!
    //! @param  params          Parameter set.
    //! @param  weight          Weight of this bxdf.
    //! @param  doubleSided     Whether the material is double sided
    MicroFacetReflection(RenderContext& rc, const ClosureTypeMirror& params, const Spectrum& weight, bool doubleSided = false);

    //! @brief Constructor from parameter set
    //!
    //! @param  params          Parameter set.
    //! @param  weight          Weight of this BRDF.
    //! @param  doubleSided     Whether the BRDF is double sided.
    MicroFacetReflection(RenderContext& rc, const ClosureTypeMicrofacetReflectionGGX &params, const Spectrum& weight , bool doubleSided = false );

    //! @brief Constructor from parameter set
    //!
    //! @param  params          Parameter set.
    //! @param  weight          Weight of this BRDF.
    //! @param  doubleSided     Whether the BRDF is double sided.
    MicroFacetReflection(RenderContext& rc, const ClosureTypeMicrofacetReflectionBlinn& params, const Spectrum& weight, bool doubleSided = false);

    //! @brief Constructor from parameter set
    //!
    //! @param  params          Parameter set.
    //! @param  weight          Weight of this BRDF.
    //! @param  doubleSided     Whether the BRDF is double sided.
    MicroFacetReflection(RenderContext& rc, const ClosureTypeMicrofacetReflectionBeckmann& params, const Spectrum& weight, bool doubleSided = false);

    //! @brief Constructor from parameter set
    //!
    //! @param  params          Parameter set.
    //! @param  weight          Weight of this BRDF.
    //! @param  doubleSided     Whether the BRDF is double sided.
    MicroFacetReflection(RenderContext& rc, const ClosureTypeMicrofacetReflectionDielectric& params, const Spectrum& weight, bool doubleSided = false);

    //! @brief Constructor
    //! @param reflectance      Direction hemisphere reflection.
    //! @param f                Fresnel term.
    //! @param d                NDF term.
    //! @param w                Weight of this BRDF
    MicroFacetReflection(RenderContext& rc, const Spectrum &reflectance, const Fresnel* f, const MicroFacetDistribution* d, const Spectrum& weight , const Vector& n , bool doubleSided = false ) :
        Microfacet(rc, d , weight , (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided) , R(reflectance), fresnel(f) {}

    //! @brief Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;

    //! @brief Importance sampling for the microfacet brdf.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;

    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override;

protected:
    const Spectrum R;                   /**< Direction-hemisphere reflection. */
    const Fresnel* fresnel = nullptr;   /**< Fresnel term. */
};

//! @brief Microfacet reflection Brdf with multi-scattering.
/**
 * Unlike the single scattering microfacet reflection model, this model also has multi-scattering.
 * This means that the brdf will be more energy conservative than the single scattering one.
 * 
 * I could have make the 'MicroFacetReflection' model energy conservative. However, I would not like
 * to adjust the behavior of Disney Brdf since it is well adjusted as the paper describes.
 * For materials that needs Multi-scattering support, they can simply use this class rather than the
 * parent one.
 */
class MicroFacetReflectionMS : public MicroFacetReflection{
public:
    //! @brief Constructor
    //! @param reflectance      Direction hemisphere reflection.
    //! @param f                Fresnel term.
    //! @param d                NDF term.
    //! @param w                Weight of this BRDF
    MicroFacetReflectionMS(RenderContext& rc, const Spectrum &reflectance, const Fresnel* f, const MicroFacetDistribution* d, const Spectrum& weight , const Vector& n , bool doubleSided = false ) :
        MicroFacetReflection(rc, reflectance, f, d, weight, n, doubleSided) {}

    //! @brief Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
};

//! @brief Microfacet Refraction BTDF.
/**
 * 'Microfacet Models for Refraction through Rough Surfaces'
 * https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
 */
class MicroFacetRefraction : public Microfacet{
public:
    //! @brief Constructor from parameter set
    //!
    //! @param  params          Parameter set.
    //! @param  f               Fresnel term.
    //! @param  weight          Weight of this BRDF.
    //! @param  doubleSided     Whether the BRDF is double sided.
    MicroFacetRefraction(RenderContext& rc, const ClosureTypeMicrofacetRefractionGGX&params,  const Spectrum& weight);

    //! @brief Constructor from parameter set
    //!
    //! @param  params          Parameter set.
    //! @param  f               Fresnel term.
    //! @param  weight          Weight of this BRDF.
    //! @param  doubleSided     Whether the BRDF is double sided.
    MicroFacetRefraction(RenderContext& rc, const ClosureTypeMicrofacetRefractionBlinn& params, const Spectrum& weight);

    //! @brief Constructor from parameter set
    //!
    //! @param  params          Parameter set.
    //! @param  f               Fresnel term.
    //! @param  weight          Weight of this BRDF.
    //! @param  doubleSided     Whether the BRDF is double sided.
    MicroFacetRefraction(RenderContext& rc, const ClosureTypeMicrofacetRefractionBeckmann& params, const Spectrum& weight);

    //! @brief Constructor
    //! @param transmittance    Direction hemisphere transmittance.
    //! @param f                Fresnel term.
    //! @param d                NDF term.
    //! @param v                Visibility term.
    //! @param etai             Index of refraction of the side that normal points
    //! @param etat             Index of refraction of the other side that normal points
    MicroFacetRefraction(RenderContext& rc, const Spectrum &transmittance, const MicroFacetDistribution* d, float etai, float etat, const Spectrum& weight , const Vector& n)
        : Microfacet(rc, d, weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_TRANSMISSION), n, true) , T(transmittance), etaI(etai) , etaT(etat) , fresnel( etai , etat )
    {
        // make sure IORs are not the same inside and outside
        if (etaT == etaI)
            etaT = etaI + 0.01f;
    }

    //! @brief Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;

    //! @brief Importance sampling for the microfacet btdf.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;

    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override;

protected:
    const Spectrum            T;          /**< Direction-hemisphere transmittance. */
    float                     etaI;       /**< Index of refraction of the side that normal points. */
    float                     etaT;       /**< Index of refraction of the other side that normal points. */
    const FresnelDielectric   fresnel;    /**< Dielectric fresnel in microfacet tranmittance model. */
};
