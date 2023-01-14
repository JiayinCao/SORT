/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.

    Copyright (c) 2011-2023 by Jiayin Cao - All rights reserved.

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
#include "scatteringevent/bsdf/bxdf.h"
#include "math/interaction.h"
#include "scatteringevent/scatteringunit.h"
#include "core/render_context.h"

class Scene;

// Up to 4 intersection supported.
#define     TOTAL_SSS_INTERSECTION_CNT      4

struct BSSRDFIntersection{
    SurfaceInteraction  intersection;
    Spectrum            weight;
};

/**
 * BSSRDFIntersection may have multiple, up to 4, intersections if needed.
 */ 
struct BSSRDFIntersections{
    BSSRDFIntersection*     intersections[TOTAL_SSS_INTERSECTION_CNT] = { nullptr };
    unsigned                cnt = 0;

    // following field is only used for spatial data structure to evaluate intersections
    float                   maxt = FLT_MAX;

    //! @brief  Resoved the maximum depth of all intersections.
    void    ResolveMaxDepth() {
        maxt = 0.0f;
        maxt = std::max(intersections[0]->intersection.t, intersections[1]->intersection.t);
        maxt = std::max(maxt, intersections[2]->intersection.t);
        maxt = std::max(maxt, intersections[3]->intersection.t);
    }
};

//! @brief BSDF implementation.
/**
 * Light sometimes penetrates through solid surfaces and bounces around beneath the surface, eventually
 * leaves the surface at another point other than the entry point. This kind of effects can usually be
 * observed in materials like wax, skin.
 */
class Bssrdf : public ScatteringUnit{
public:
    //! @brief  Default constructor
    //!
    //! @param  ew      Evaluation weight.
    //! @param  sw      Sample weight.
    Bssrdf( const Spectrum& ew , const float sw ) : ScatteringUnit( ew , sw ){}

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
    //! @param  inter   Incident intersection sampled.
    virtual void        Sample_S( const Scene& scene , const Vector& wo , const Point& po , BSSRDFIntersections& inter, RenderContext& rc ) const = 0;
};

//! @brief  Separable BSSRDF implementation.
/**
 * BSSRDF is usually very difficult to evaluate due to its eight dimensional nature. It is true for even simplest
 * cases like a slab. It is not uncommon to see BSSRDF implementation with lots of gross approximations to simplify
 * things so that it is tractable.
 * Separable BSSRDF is one of them by making the following assumptions.
 *   - BSSRDF can be separate with three different components, two of which are directional and the other one is spatial.
 *   - The spatial term purely depends on the distance between the incident and extant positions, this may break in cases of complex shapes.
 */
class SeparableBssrdf : public Bssrdf{
public:
    //! @brief   Constructor.
    //!
    //! @param  R               Base color of the bssrdf.
    //! @param  intersection    Intersection of bssrdf, usually exit point.
    //! @param  ew              Evaluation weight.
    //! @param  sw              Sample weight.
    SeparableBssrdf( const Spectrum& R , const SurfaceInteraction* intersection , const Spectrum& ew , const float sw );

    //! @brief  Importance sample the incident position.
    //!
    //! The exact importance sampling algorithm comes from this paper.
    //! BSSRDF Importance Sampling
    //! http://library.imageworks.com/pdfs/imageworks-library-BSSRDF-sampling.pdf
    //!
    //! @param  scene   The scene where ray tracing happens.
    //! @param  wo      Extant direction.
    //! @param  po      Extant position.
    //! @param  inter   Intersection between the rays and the objects with same material.
    void        Sample_S( const Scene& scene , const Vector& wo , const Point& po , BSSRDFIntersections& inter, RenderContext& rc) const override;

    //! @brief  PDF of sampling the reflectance profile.
    //!
    //! @param  po      Extant position in world coordinate.
    //! @param  pi      Incident position in world coordinate.
    //! @param  n       Normal in world coordinate at incident position.
    //! @return         Pdf of sampling the distance based on the reflectance profile.
    float       Pdf_Sp( const Point& po , const Point& pi , const Vector& n ) const;

protected:
    //! @brief  Evaluate the reflectance profile based on distance between the two points.
    //!
    //! @param  distance    Distance between the incident and extant positions.
    //! @return             Reflectance profile based on the distance.
    virtual Spectrum    Sr( float distance ) const = 0;

    //! @brief  Sampling a distance based on the reflectance profile.
    //!
    //! @param  ch      Spectrum channel.
    //! @param  r       A canonical value used to randomly sample distance
    //! @return         The distance sampled. A negative returned value means invalid sample.
    virtual float       Sample_Sr(int ch, float r) const = 0;

    //! @brief  Pdf of sampling such a distance based on the reflectance profile.
    //!
    //! @param  ch      Spectrum channel.
    //! @param  d       Distance from the extant point.
    //! @return         Pdf of sampling it.
    virtual float       Pdf_Sr(int ch, float d) const = 0;

    //! @brief  Get maximum profile sampling distance
    //!
    //! @param  ch      Spectrum channel of interest. The returned distance sometimes depends on spectrum channel.
    //! @return         Maximum profile sampling distance.
    virtual float       Max_Sr( int ch ) const = 0;

    //! @brief  Sample a channel
    //!
    //! @return         Randomly pick a channel in spectrum, in which the mean free path is not zero.
    virtual int         Sample_Ch(RenderContext& rc) const = 0;

    Vector nn;          /**< Normal at the point to be Evaluated. */
    Vector btn;         /**< Bi-tangent at the point to be evaluated. */
    Vector tn;          /**< Tangent at the point to be Evaluated. */

    int    channels;    /**< Number of channels in spectrum in which mfp is not zero. */
    Spectrum    R;      /**< Reflectance of the BSSRDF. */

    const SurfaceInteraction*   intersection;   /**< Intersection that spawns the BSSRDF. */
};