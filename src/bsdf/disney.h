/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2019 by Cao Jiayin - All rights reserved.
 
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
#include "microfacet.h"
#include "bssrdf/bssrdf.h"

class Bssrdf;

//! @brief Disney Principle BRDF.
/**
 * Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering
 * http://blog.selfshadow.com/publications/s2015-shading-course/burley/s2015_pbs_disney_bsdf_notes.pdf
 *
 * Physically Based Shading at Disney
 * https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
 *
 * The initial implementation was based on the open-source Github project from Walt Disney Animation Studios
 * https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
 *
 * Rendering the Moana Island Scene Part 1: Implementing the Disney BSDF
 * https://schuttejoe.github.io/post/disneybsdf/
 */
class DisneyBRDF : public Bxdf{
public:
    // Input parameters to construct the disney BRDF.
    struct Params {
        float metallic;
        float specular;
        float specularTint;
        float roughness;
        float anisotropic;
        float sheen;
        float sheenTint;
        float clearcoat;
        float clearcoatGloss;
        float specTrans;
        float scatterDistance;
        float flatness;
        float diffTrans;
        int   thinSurface;
        RGBSpectrum baseColor;
        Vector n;
    };

    //! Constructor
    //!
    //! @param params           All parameters.
    //! @param bssrdf           BSSRDF that may be filled depending on the parameter setup.
    //! @param weight           Weight of the BXDF
    DisneyBRDF( const Params& param , Bssrdf*& bssrdf , const Spectrum& weight)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), param.n, true), basecolor(param.baseColor), metallic(param.metallic),
        specular(param.specular), specularTint(param.specularTint), roughness(param.roughness), anisotropic(param.anisotropic), sheen(param.sheen), sheenTint(param.sheenTint),
        clearcoat(param.clearcoat), clearcoatGloss(param.clearcoatGloss), specTrans(param.specTrans), scatterDistance(param.scatterDistance), flatness(param.flatness), 
          diffTrans(param.diffTrans), thinSurface( param.thinSurface != 0 ), bssrdf(bssrdf) {}

	//! Constructor
    //! @param basecolor        The surface color, usually supplied by texture maps.
    //! @param metallic         This is a linear blend between two different models. The metallic model has no diffuse component and also has a tinted incident specular, equal to the base color.
    //! @param specular         Direction-hemisphere reflection for specular.
    //! @param speculatTint     A concession for artistic control that tints incident specular towards the base color.
    //! @param roughness        Surface roughness, controls both diffuse and specular response.
    //! @param anisotropic      Degree of anisotropy. This controls the aspect ratio of the specular highlight. (0 = isotropic, 1 = maximally anisotropic.)
    //! @param sheen            An additional grazing component, primarily intended for cloth.
    //! @param sheenTint        Amount to tint sheen towards base color.
    //! @param clearcoat        A second, special-purpose specular lobe.
    //! @param clearcoatGloss   Controls clearcoat glossiness (0 = a “satin” appearance, 1 = a “gloss” appearance)
    //! @param specTrans        A blending factor controlling how transparent the surface is.
    //! @param scatterDistance  The distance of scattering beneath the surface, a positive value indicates there is SSS.
    //! @param flatness         A factor for blending diffuse and fakeSS for thin surface.
    //! @param diffTrans        A blending factor for diffuse transmission and reflectance model.
    //! @param thinSurface      Whether the surface is a thin surface.
    //! @param bssrdf           BSSRDF that may be filled depending on the parameter setup.
    //! @param weight           Weight of the BXDF.
    //! @param n                Normal in shading coordinate.
    DisneyBRDF( const Spectrum& basecolor , float metallic , float specular , float specularTint , float roughness ,
               float anisotropic , float sheen , float sheenTint , float clearcoat , float clearcoatGloss , float specTrans , float scatterDistance , 
               float flatness , float diffTrans , int thinSurface , Bssrdf*& bssrdf , const Spectrum& weight, const Vector& n )
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, true) , basecolor(basecolor), metallic(metallic),
          specular(specular), specularTint(specularTint), roughness(roughness), anisotropic(anisotropic), sheen(sheen), sheenTint(sheenTint),
          clearcoat(clearcoat), clearcoatGloss(clearcoatGloss), specTrans(specTrans), scatterDistance(scatterDistance), flatness(flatness), 
          diffTrans(diffTrans), thinSurface( thinSurface != 0 ), bssrdf(bssrdf) {}
	
    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
	
    //! @brief Importance sampling for the fresnel brdf.
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
    
private:
	const Spectrum  basecolor;          /**< The surface color, usually supplied by texture maps. */
    const float     metallic;           /**< The metallic-ness (0 = dielectric, 1 = metallic). This is a linear blend between two different models. The metallic model has no diffuse component and also has a tinted incident specular, equal to the base color. */
    const float     specular;           /**< Incident specular amount. This is in lieu of an explicit index-of-refraction. */
    const float     specularTint;       /**< A concession for artistic control that tints incident specular towards the base color. Grazing specular is still achromatic. */
    const float     roughness;          /**< Surface roughness, controls both diffuse and specular response. */
    const float     anisotropic;        /**< degree of anisotropy. This controls the aspect ratio of the specular highlight. (0 = isotropic, 1 = maximally anisotropic). */
    const float     sheen;              /**< An additional grazing component, primarily intended for cloth. */
    const float     sheenTint;          /**< Amount to tint sheen towards base color. */
    const float     clearcoat;          /**< A second, special-purpose specular lobe. */
    const float     clearcoatGloss;     /**< controls clearcoat glossiness (0 = a “satin” appearance, 1 = a “gloss” appearance). */
    const float     specTrans;          /**< Specular Transmission. */
    const float     diffTrans;          /**< Diffuse Transmission. */
    const float     flatness;           /**< Blending factor between diffuse and fakeSS model. */
    const float     scatterDistance;    /**< Distance of scattering in SSS. */
    const bool      thinSurface;        /**< Whether the surface is thin surface. */
    Bssrdf*&        bssrdf;             /**< BSSRDF may be filled by the brdf. */

    //! @brief      Get R0 with relative IOR.
    //!
    //! Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering, section 3.1
    //!
    //! @param rROI     Relative index of refraction, IOR_ouside / IOR_inside
    //! @return         Evaluated R0 for Fresnel
    inline float SchlickR0FromEta( float rROI ) const{
        return SQR( ( rROI - 1.0f ) / ( rROI + 1.0f ) );
    }
};

//! @brief  Clearcoat GGX NDF.
class ClearcoatGGX : public GGX{
public:
    //! @brief Constructor
    //! @param roughness    Roughness of the surface formed by the micro facets.
    ClearcoatGGX(float roughness) : GGX(roughness, roughness) {}

    //! @brief probability of facet with specific normal (h)
    float D(const Vector& h) const override;

    //! @brief Sampling a normal respect to the NDF.
    //!
    //! @param bs   Sample holding all necessary random variables.
    //! @return     Sampled normal direction based on the NDF.
    Vector sample_f(const BsdfSample& bs) const override;

protected:
    //! @brief Smith shadow-masking function G1
    float G1(const Vector& v) const override;
};

//! @brief  DisneyBssrdf implementation.
/**
 * Approximate Reflectance Profiles for Efficient Subsurface Scattering
 * https://graphics.pixar.com/library/ApproxBSSRDF/paper.pdf
 */
class DisneyBssrdf : public SeparableBssrdf {
public:
    //! @brief  Constructor of DisneyBssrdf.
    //!
    //! @param  intersect   Intersection information at exitant point.
    //! @param  R           Reflectance of the material.
    //! @param  dd          Spectrum dependent mean distance.
    //! @param  ior_i       Index of refraction inside the surface.
    //! @param  ior_e       Index of refraction outside the surface.
    DisneyBssrdf( const Intersection* intersect , const Spectrum& R , const Spectrum& dd , float ior_i , float ior_e );

    //! @brief  Evalute the reflectance profile based on distance between the two points.
    //!
    //! @param  distance    Distance between the incident and extant positions.
    //! @return             Reflectance profile based on the distance.
    Spectrum    Sr( float distance ) const override;

    //! @brief  Sampling a distance based on the reflectance profile.
    //!
    //! @param  ch      Spectrum channel.
    //! @param  r       A canonical value used to randly sample distance
    //! @return         The distance sampled.
    float       Sample_Sr(int ch, float r) const override;

    //! @brief  Pdf of sampling such a distance based on the reflectance profile.
    //!
    //! @param  ch      Spectrum channel.
    //! @param  d       Distance from the extant point.
    //! @return         Pdf of sampling it.
    float       Pdf_Sr(int ch, float d) const override;

    //! @brief  Evaluate the BSSRDF.
    //!
    //! Unlike BXDF, BSSRDF is more of a generalized version function of eight dimensions.
    //!
    //! @param  wo      Extant direction.
    //! @param  po      Extant position.
    //! @param  wi      Incident direction.
    //! @param  pi      Incident position.
    //! @return         To be figured out
    Spectrum    S( const Vector& wo , const Point& po , const Vector& wi , const Point& pi ) const override;

private:
    Spectrum    R;
    Spectrum    d;
};