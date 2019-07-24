/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2019 by Jiayin Cao - All rights reserved.

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

#include "core/define.h"
#include "spectrum/spectrum.h"
#include "math/vector3.h"
#include "math/point.h"
#include "bsdf/bxdf.h"

class Intersection;
class Scene;
class Bsdf;

//! @brief BSDF implementation.
/**
 * Light sometimes penetrates through solid surfaces and bounces around beneath the surface, evetually
 * leaves the surface at another point other than the entry point. This kind of effects can usually be
 * observed in materials like wax, skin.
 */
class Bssrdf{
public:
    //! @brief  Constructor
    //!
    //! @param  ior_i       Index of refraction beneath the surface.
    //! @param  ior_o       Index of refraction above the surface.
    Bssrdf( const float ior_i , const float ior_e ):ior_i(ior_i),ior_e(ior_e){}

    //! @brief  Empty default destructor
    virtual ~Bssrdf() = default;

    //! @brief  Evaluate the BSSRDF.
    //!
    //! Unlike BXDF, BSSRDF is more of a generalized version function of eight dimensions.
    //!
    //! @param  wo      Extant direction.
    //! @param  po      Extant position.
    //! @param  wi      Incident direction.
    //! @param  pi      Incident position.
    //! @param  pdf     Pdf of sampling such a point on the surface of the object.
    //! @return         To be figured out
    virtual Spectrum    S( const Vector& wo , const Point& po , const Vector& wi , const Point& pi ) const = 0;

    //! @brief  Importance sample the incident direction and position.
    //!
    //! @param  scene   The scene where ray tracing happens.
    //! @param  wo      Extant direction.
    //! @param  po      Extant position.
    //! @param  wi      Incident direction to be updated.
    //! @param  pi      Incident position to be updated.
    //! @param  pdf     Pdf of sampling such a point on the surface of the object.
    //! @param  bsdf    The bsdf at the incident intersection.
    //! @return         To be figured out
    virtual Spectrum    Sample_S( const Scene& scene , const Vector& wo , const Point& po , Vector& wi , Point& pi , float& pdf , Bsdf*& bsdf ) const = 0;

protected:
    const float ior_i;  /**< Index of refraction inside the surface. */
    const float ior_e;  /**< Index of refraction outside the surface. */
};

//! @brief  Separable BSSRDF implementation.
/**
 * BSSRDF is usually very difficult to evaluate due to its eight dimensional nature. It is true for even simplest
 * cases like a slab. It is not uncommon to see BSSRDF implementation with lots of gross approximations to simplify
 * things so that it is tractable.
 * Separable BSSRDF is one of them by making the following assumptions.
 *   - BSSRDF can be seperate with three different components, two of which are directional and the other one is spatial.
 *   - The spatial term purely depends on the distance between the incident and extant positions, this may break in cases of complex shapes.
 */
class SeparableBssrdf : public Bssrdf{
public:
    SeparableBssrdf( const Intersection* intersection , const float ior_i , const float ior_e );

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

    //! @brief  Importance sample the incident direction and position.
    //!
    //! @param  scene   The scene where ray tracing happens.
    //! @param  wo      Extant direction.
    //! @param  po      Extant position.
    //! @param  wi      Incident direction to be updated.
    //! @param  pi      This will be ignored.
    //! @param  pdf     Pdf of sampling such a point on the surface of the object.
    //! @param  bsdf    The bsdf at the incident intersection.
    //! @return         The spatial term in separable Bssrdf multiplied by the fresnel term of the incident direction.
    Spectrum    Sample_S( const Scene& scene , const Vector& wo , const Point& po , Vector& wi , Point& pi , float& pdf , Bsdf*& bsdf ) const override;

    //! @brief  Importance sample the incident position.
    //!
    //! @param  scene   The scene where ray tracing happens.
    //! @param  wo      Extant direction.
    //! @param  po      Extant position.
    //! @param  pi      Incident position to be updated.
    //! @param  pdf     Pdf of sampling such a point on the surface of the object.
    //! @return         The spatial term in separable Bssrdf.
    Spectrum    Sample_Sp( const Scene& scene , const Vector& wo , const Point& po , Point& pi , float& pdf ) const;

    //! @brief  PDF of sampling the reflectance profile.
    //!
    //! @param  po      Extant position in world coordinate.
    //! @param  pi      Incident position in world coordinate.
    //! @param  n       Normal in world coordinate at incident position.
    //! @return         Pdf of sampling the distance based on the reflectance profile.
    float       Pdf_Sp( const Point& po , const Point& pi , const Vector& n ) const;

protected:
    //! @brief  Evalute the reflectance profile based on distance between the two points.
    //!
    //! @param  distance    Distance between the incident and extant positions.
    //! @return             Reflectance profile based on the distance.
    virtual Spectrum    Sr( float distance ) const = 0;

    //! @brief  Sampling a distance based on the reflectance profile.
    //!
    //! @param  ch      Spectrum channel.
    //! @param  r       A canonical value used to randly sample distance
    //! @return         The distance sampled.
    virtual float       Sample_Sr(int ch, float r) const = 0;

    //! @brief  Pdf of sampling such a distance based on the reflectance profile.
    //!
    //! @param  ch      Spectrum channel.
    //! @param  d       Distance from the extant point.
    //! @return         Pdf of sampling it.
    virtual float       Pdf_Sr(int ch, float d) const = 0;

    Vector nn;      /**< Normal at the point to be Evaluated. */
    Vector btn;     /**< Bi-tangent at the point to be evaluated. */
    Vector tn;      /**< Tangent at the point to be Evaluated. */

    //! @brief  One of the directional components of separable Bssrdf.
    //!
    //! @param  wi      Incident direction.
    //! @return         This is usually related for fresnel.
    Spectrum    Sw( const Vector& wi ) const;

    friend class SeparableBssrdfAdapter;
};

class SeparableBssrdfAdapter : public Bxdf {
public:
    //! @brief  Constructor taking a bssrdf.
    //!
    //! @param  bssrdf      Bssrdf that the adapter is used in.
    SeparableBssrdfAdapter( const SeparableBssrdf* bssrdf );

    //! Evalute the BXDF.
    //!
    //! @param  wo      The exitant direction in local space.
    //! @param  wi      The incident direction in local space.
    //! @return         Evaluted BRDF by cos(\theta)
    //Spectrum F( const Vector& wo , const Vector& wi ) const override;

    //! @brief Evaluate the BRDF.
    //!
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;

private:
    const SeparableBssrdf*  m_bssrdf;
};